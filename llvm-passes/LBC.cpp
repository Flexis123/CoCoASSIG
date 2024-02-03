#define DEBUG_TYPE "BoundsChecker"
#include "utils.h"
#include <map>

namespace {
    class BoundsChecker : public ModulePass {
    public:
        static char ID;
        BoundsChecker() : ModulePass(ID) {}
        virtual bool runOnModule(Module &M) override;

    private:
        DenseMap<Function*, DenseMap<int, int>> fnClones;
        DenseMap<PHINode*, PHINode*> annotatedPhis;
        Value* newArgc = nullptr;

        Value* accumulatedOffsetOf(Module &M, GetElementPtrInst* gep);
        inline bool isConstPtr(Value* v, Value** size = nullptr);
        inline bool isOrigin(Value* v);
        Value &getPointerOrigin(Value* v);

        Value* getPointerElemNum(Value &v);
        PHINode* addSizePhi(PHINode* phi);

    };
}

Value* BoundsChecker::accumulatedOffsetOf(Module &M, GetElementPtrInst* gep){
    IntegerType* t = Type::getInt32Ty(M.getContext());

    APInt constOffsetVal = ConstantInt::get(t, 0, true)->getValue();
    Value* res = nullptr;
    IRBuilder<> B(gep);
    GetElementPtrInst* currGep = gep;
    do{
        if(currGep->getNumOperands() == 2){

            Value* currOffset = currGep->getOperand(1);

            if(ConstantInt* off = dyn_cast<ConstantInt>(currOffset)) constOffsetVal += off->getValue();
            else if(res) res = B.CreateAdd(res, currOffset);
            else res = currOffset;
        }
    }while((currGep = dyn_cast<GetElementPtrInst>(currGep->getOperand(0))));

    ConstantInt* constOffset = cast<ConstantInt>(ConstantInt::get(t, constOffsetVal));
    if(res && !constOffset->isZero()) return B.CreateAdd(res, constOffset);
    else if(res) return res;
    return constOffset;
}

inline bool BoundsChecker::isConstPtr(Value* v, Value** size){
    Type *Int32Ty = Type::getInt32Ty(v->getContext());
    if(PointerType* ptr = dyn_cast<PointerType>(v->getType())){
        if(ArrayType *arr = dyn_cast<ArrayType>(ptr->getElementType())){
            if(size) *size = ConstantInt::get(Int32Ty, arr->getNumElements(), true);

            return true;
        }
    }
    return false;
}

inline bool BoundsChecker::isOrigin(Value* v){
    return dyn_cast<LoadInst>(v) || dyn_cast<AllocaInst>(v) || isConstPtr(v) || dyn_cast<Argument>(v) 
            || dyn_cast<CallInst>(v) || dyn_cast<PHINode>(v);
}

Value& BoundsChecker::getPointerOrigin(Value* v){
    Instruction *I;
    while (!isOrigin(v)){
        if((I = dyn_cast<Instruction>(v))){
            for(Use &U: I->operands()){
                v = U.get();
                if(v->getType()->isPointerTy()) break;
            }
        }else ERROR("Error: pointer has unsupported origin\n");
    }

    return *v;
}


Value* BoundsChecker::getPointerElemNum(Value &v){
    Value *V = &v;
    Argument* arg;
    Function* f;
    Value* constSize;

    Type *Int32Ty = Type::getInt32Ty(v.getContext());

    //deal with constants and loads and calls (load and calls assume max size 8192)
    if(AllocaInst* alloca = dyn_cast<AllocaInst>(V)) {
        return alloca->getOperand(0);
    }
    else if(dyn_cast<LoadInst>(V) || dyn_cast<CallInst>(V)) {
        return ConstantInt::get(Int32Ty, 8192, true);
    }
    else if((arg = dyn_cast<Argument>(V)) && arg->getType()->isPointerTy() && (f = arg->getParent())){
        if(f->getName().equals("main")) return newArgc;
        else{
            auto it = fnClones.find(f);
            if(it != fnClones.end()) {
                DenseMap<int, int> argSizeMap = (*it).second;
                
                auto it2 = argSizeMap.find(arg->getArgNo());
                if(it2 != argSizeMap.end()) return f->getArg((*it2).second);
            }
        }
    } 
    else if(isConstPtr(V, &constSize)) return constSize;
    else if(PHINode* phi = dyn_cast<PHINode>(V)) {
        auto annotattedPhiEntry = annotatedPhis.find(phi);
        if(annotattedPhiEntry != annotatedPhis.end()) return (*annotattedPhiEntry).second;
    }
    
    return NULL;
}


PHINode* BoundsChecker::addSizePhi(PHINode* phi){
    LOG_LINE("phi = " << phi);
    auto annotatedPhiEntry = annotatedPhis.find(phi);
    
    if(annotatedPhiEntry != annotatedPhis.end()) return (*annotatedPhiEntry).second;
    
    IRBuilder<> B(phi->getNextNode());
    PHINode* sizePhi = B.CreatePHI(Type::getInt32Ty(phi->getContext()), phi->getNumOperands(), phi->getName() + ".size");

    annotatedPhis.try_emplace(phi, sizePhi);
     
    SmallVector<Value*, 5> sizes;
    for(Use& U: phi->incoming_values()){
        Value* v = U.get();

        if(PHINode* inPhi = dyn_cast<PHINode>(v)) sizes.push_back(addSizePhi(inPhi));
        else if(Value* elemNum = getPointerElemNum(getPointerOrigin(v))) sizes.push_back(elemNum);
        else ERROR("Error when annotating pointer phi node, couldnt find pointer size");
    }

    for(unsigned i = 0; i < sizes.size(); i++) {
        sizePhi->addIncoming(sizes[i], phi->getIncomingBlock(i));
    }

    return sizePhi;
}


bool BoundsChecker::runOnModule(Module &M) {
    LLVMContext &ctx = M.getContext();
    Type *Int32Ty = Type::getInt32Ty(ctx);
    Type *VoidTy = Type::getVoidTy(ctx);
    Function* bounds_check_fn = cast<Function>(M.getOrInsertFunction("__coco_check_bounds", VoidTy, Int32Ty, Int32Ty).getCallee());

    //change argc for main to be argc + 1;
    //augmentArgc(M);
    if(Function* main = M.getFunction("main")){
        IRBuilder<> B(&*main->getEntryBlock().getFirstInsertionPt());
        newArgc = B.CreateAdd(main->getArg(0), ConstantInt::get(Type::getInt32Ty(M.getContext()), 1));
    }

    DenseMap<Function*, Function*> clonedFns;

    bool changed = false;
    for (Function &F : M) {

        if(!shouldInstrument(&F) || F.getName().equals("main"))
            continue;
        

        DenseMap<int, int> ptrSize;
        SmallVector<Type*, 5> newArgsTypes;
    
        //find pointer arguments
        unsigned pointers = 0;
        for(Argument &arg: F.args()){
            if(arg.getType()->isPointerTy()){
                ptrSize.try_emplace(arg.getArgNo(), pointers + F.arg_size());
                pointers++;

                newArgsTypes.push_back(Int32Ty);
            }
        }
    
        changed |= !ptrSize.empty();

        if(!ptrSize.empty()){

            //clone function
            SmallVector<Argument*, 10> cloneArgs;
            Function* cloneFn = addParamsToFunction(&F, ArrayRef<Type*>(newArgsTypes), cloneArgs);
            fnClones.try_emplace(cloneFn, ptrSize);

            //add cloned fn
            clonedFns.try_emplace(&F, cloneFn); 
        }
    }

    //here do phi stuff
    DenseMap<PHINode*, PHINode*> annotatedPhis;
    for(Function &F : M){
        if (!shouldInstrument(&F))
            continue;

        SmallVector<PHINode*, 8> ptrPhis;
        PHINode* phi;
        for(Instruction &I: instructions(F)){
            if((phi = dyn_cast<PHINode>(&I)) && phi->getType()->isPointerTy()) ptrPhis.push_back(phi);
        }

        for(PHINode* phi : ptrPhis) addSizePhi(phi);
    }

    for (Function &F : M) {
        if (!shouldInstrument(&F))
            continue;


        SmallVector<std::pair<Instruction*, Instruction*>, 10> toRepalce;
        for(Instruction &I: instructions(F)){
            if(CallInst* call = dyn_cast<CallInst>(&I)){
                auto fCloneEntry = clonedFns.find(call->getCalledFunction());

                if(fCloneEntry != clonedFns.end() && clonedFns.find(call->getFunction()) == clonedFns.end()){
                    Function* cloneF = (*fCloneEntry).second;

                    SmallVector<Value*, 5> sizes;
                    for(Use &U: call->args()){
                        Value* v = U.get();
                        Value* elemNum;

                        if(v->getType()->isPointerTy() && (elemNum = getPointerElemNum(getPointerOrigin(v)))){
                            sizes.push_back(elemNum);
                        }
                    }

                    SmallVector<Value*, 10> cloneArgs;
                    for(Use &U: call->args()) cloneArgs.push_back(U.get());
                    for(Value* size: sizes) cloneArgs.push_back(size);
                    
 
                    CallInst* cloneCall = CallInst::Create(cloneF->getFunctionType(), cloneF, ArrayRef<Value*>(cloneArgs));
                    toRepalce.push_back(std::make_pair(call, cloneCall));
                }
            }
        }

        for(auto rep: toRepalce) ReplaceInstWithInst(rep.first, rep.second);
    }

    //removing cloned functions
    for(auto& elem: clonedFns) {
        SmallVector<Instruction*, 10> toRemove;
        for(Instruction &I : instructions(*elem.first)){
            if(CallInst* call = dyn_cast<CallInst>(&I)) toRemove.push_back(call);
        }

        for(Instruction* I : toRemove) I->eraseFromParent();
    }

    for(auto& elem: clonedFns) elem.first->eraseFromParent();

    for (Function &F : M) {
        if (!shouldInstrument(&F))
            continue;

        //mutate function calls to pass pointer size

        //introduce bound check 
        GetElementPtrInst *gep;
        for(Instruction &I: instructions(F)){
            if((gep = dyn_cast<GetElementPtrInst>(&I)) && gep->getNumOperands() < 3){
                Value* accOffset = accumulatedOffsetOf(M, gep);
                Value &orig = getPointerOrigin(gep);
                Value* arraySize = getPointerElemNum(orig);

                changed |= accOffset && arraySize;

                if(accOffset && arraySize){
                    IRBuilder<> B(gep);
                    B.CreateCall(bounds_check_fn, {accOffset, arraySize});
                }
            }
        }
    
    }

    //delete all cloned functions

    return changed;
}

// Register the pass with LLVM so we can invoke it with opt. The first argument
// to RegisterPass is the commandline switch to run this pass (e.g., opt
// -coco-BoundsChecker, the second argument is a description shown in the help
// text about this pass.
char BoundsChecker::ID = 4;
static RegisterPass<BoundsChecker> X("coco-boundscheck", "Bound check instrumentation");
