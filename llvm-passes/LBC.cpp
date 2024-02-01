#define DEBUG_TYPE "BoundsChecker"
#include "utils.h"

namespace {
    class BoundsChecker : public ModulePass {
    public:
        static char ID;
        BoundsChecker() : ModulePass(ID) {}
        virtual bool runOnModule(Module &M) override;

    private:
    };
}

class FunctionModificationInfo{
    public:
        DenseMap<int, int> argSizeMap;
        ArrayRef<llvm::Type*> newParamTypes;

        FunctionModificationInfo(DenseMap<int, int> argSizeMap, ArrayRef<llvm::Type *> newParamTypes){
            this->argSizeMap = argSizeMap;
            this->newParamTypes = newParamTypes;
        }

        FunctionModificationInfo(){}
};

Value* accumulatedOffsetOf(Module &M, GetElementPtrInst* gep){
    IntegerType* t = Type::getInt32Ty(M.getContext());

    APInt constOffsetVal = ConstantInt::get(t, 0, true)->getValue();
    Value* res = nullptr;
    IRBuilder<> B(gep);
    GetElementPtrInst* currGep = gep;
    do{
        if(currGep->getNumOperands() > 2) return NULL;
        else if(currGep->getNumOperands() == 2){

            Value* currOffset = currGep->getOperand(1);

            if(ConstantInt* off = dyn_cast<ConstantInt>(currOffset)) constOffsetVal += off->getValue();
            else if(res) res = B.CreateAdd(res, currOffset);
            else res = currOffset;
        }
    }while((currGep = dyn_cast<GetElementPtrInst>(currGep->getOperand(0))));

    ConstantInt* constOffset = cast<ConstantInt>(ConstantInt::get(t, constOffsetVal));
    if(res && !constOffset->isZero()) return B.CreateAdd(res, constOffset);
    return constOffset;
}

inline bool isConstPtr(Value* v){
    return dyn_cast<ConstantInt>(v) && v->getType()->isPointerTy();
}
/*
    TODO CHECK
 If, your code encounters any unknown type of origin, it should raise an error.
*/
Value &getPointerOrigin(Instruction* gep){
    Instruction* I = gep;

    while (true){
        bool foundNext = false;

        for(Use &U: I->operands()){
            Value *v = U.get();

            if(dyn_cast<LoadInst>(v) || dyn_cast<AllocaInst>(v) || isConstPtr(v) || dyn_cast<Argument>(v) || dyn_cast<CallInst>(v))  return *v;
            else if((I = dyn_cast<Instruction>(v)) && I->getType()->isPointerTy()) {
                foundNext = true;
                break;
            }
        }

        assert(foundNext);
    }
}

Value* getPointerElemNum(Value &v,  DenseMap<Function*, FunctionModificationInfo> &clonedFnArgSizeMap){
    Value *V = &v;
    Argument* arg;
    Function* f;

    if(AllocaInst* alloca = dyn_cast<AllocaInst>(V)) return alloca->getOperand(0);
    else if(isConstPtr(V)) return V;
    else if((arg = dyn_cast<Argument>(V)) && arg->getType()->isPointerTy() && (f = arg->getParent())){
        if(f->getName().equals("main")) return f->getArg(0); // todo make argc + 1 if needed
        else{
            auto it = clonedFnArgSizeMap.find(f);
            if(it != clonedFnArgSizeMap.end()) {
                DenseMap<int, int> argSizeMap = (*it).second.argSizeMap;
                
                auto it2 = argSizeMap.find(arg->getArgNo());
                if(it2 != argSizeMap.end()) return f->getArg((*it2).second);
            }
        }
    } 
    
    return NULL;
}

bool BoundsChecker::runOnModule(Module &M) {
    LLVMContext &ctx = M.getContext();
    Type *Int32Ty = Type::getInt32Ty(ctx);
    Type *VoidTy = Type::getVoidTy(ctx);
    Function* bounds_check_fn = cast<Function>(M.getOrInsertFunction("__coco_check_bounds", VoidTy, Int32Ty, Int32Ty).getCallee());

    DenseMap<Function*, FunctionModificationInfo> clonedFnArgSizeMap;

    bool changed = false;
    for (Function &F : M) {
        if (!shouldInstrument(&F))
            continue;

        for(Instruction &I: instructions(F)){
            if(CallInst* call = dyn_cast<CallInst>(&I)){

                Function* fn = call->getFunction();
                FunctionModificationInfo fnInf;
                auto argSizeMapEntry = clonedFnArgSizeMap.find(fn);

                if(argSizeMapEntry == clonedFnArgSizeMap.end()){

                    DenseMap<int, int> argSizeMap = DenseMap<int, int>();
                    SmallVector<Type*, 5> newTypes;
                    unsigned sizeIndex = 0;

                    for(Use &U: call->operands()){
                        Value* v = U.get();
                        Instruction* II;
                        Value* elemNum;
                        if((dyn_cast<Instruction>(v)) && II->getType()->isPointerTy()){
                            if(Value* elemNum = getPointerElemNum(getPointerOrigin(II), clonedFnArgSizeMap)){
                                argSizeMap.try_emplace(U.getOperandNo(), sizeIndex);
                                sizeIndex++;
                            }
                        }
                    }

                    fnInf = FunctionModificationInfo(argSizeMap, ArrayRef<Type*>(newTypes));
                    clonedFnArgSizeMap.try_emplace(fn, argSizeMap);
                }

                SmallVector<Argument*, 5> newArgs;
                Function* newFn = addParamsToFunction(fn, fnInf.newParamTypes, newArgs);
                ReplaceInstWithInst(call, CallInst::Create(fn));


            }
        }

        GetElementPtrInst *gep;
        for(Instruction &I: instructions(F)){
            if((gep = dyn_cast<GetElementPtrInst>(&I)) && gep->getNumOperands() < 3){
                Value* accOffset = accumulatedOffsetOf(M, gep);
                Value &orig = getPointerOrigin(gep);
                Value* arraySize = getPointerElemNum(orig, clonedFnArgSizeMap);

                //todo check if orig is argc
            
                changed |= accOffset && arraySize;

                if(changed){
                    IRBuilder<> B(gep);
                    B.CreateCall(bounds_check_fn, {accOffset, arraySize});
                }
            }
        }
    
    }

    return changed;
}

// Register the pass with LLVM so we can invoke it with opt. The first argument
// to RegisterPass is the commandline switch to run this pass (e.g., opt
// -coco-BoundsChecker, the second argument is a description shown in the help
// text about this pass.
char BoundsChecker::ID = 4;
static RegisterPass<BoundsChecker> X("coco-boundscheck", "Bound check instrumentation");
