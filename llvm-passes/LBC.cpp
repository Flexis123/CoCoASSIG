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

Value* accumulatedOffsetOf(Module &M, GetElementPtrInst* gep){
    IntegerType* t = Type::getInt32Ty(M.getContext());
    APInt offsetVal = ConstantInt::get(t, 0, true)->getValue();

    ConstantInt* currOfsset;
    do{
        if(gep->getNumOperands() > 1 && !(currOfsset = dyn_cast<ConstantInt>(gep->getOperand(1)))){
            return NULL;
        }else if(gep->getNumOperands() > 1){
            offsetVal += currOfsset->getValue();
        }
    }while((gep = dyn_cast<GetElementPtrInst>(gep->getOperand(0))));
    
    return ConstantInt::get(t, offsetVal);
}

inline bool isConstPtr(Value* v){
    return dyn_cast<ConstantInt>(v) && v->getType()->isPointerTy();
}
/*
    TODO CHECK
 If, your code encounters any unknown type of origin, it should raise an error.
*/
Value &getPointerOrigin(GetElementPtrInst* gep){
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

Value* getPointerElemNum(Value &v){
    Value *V = &v;
    Argument* arg;
    Function* f;

    if(AllocaInst* alloca = dyn_cast<AllocaInst>(V)) return alloca->getOperand(0);
    else if(isConstPtr(V)) return V;
    else if((arg = dyn_cast<Argument>(V)) && (f = arg->getParent()) && f->getName().equals("main")){
        return f->getArg(0); //ARGC
    }
    return NULL;
}

bool BoundsChecker::runOnModule(Module &M) {
    Type *Int32Ty = Type::getInt32Ty(M.getContext());
    Function* bounds_check_fn = cast<Function>(M.getOrInsertFunction("__coco_check_bounds", Int32Ty, Int32Ty).getCallee());

    bool changed = false;
    for (Function &F : M) {
        if (!shouldInstrument(&F))
            continue;

        IRBuilder<> B(&F.getEntryBlock());

        GetElementPtrInst *gep;
        for(BasicBlock &BB : F){
            for(Instruction &I: BB){
                if((gep = dyn_cast<GetElementPtrInst>(&I)) && gep->getNumOperands() < 3){
                    Value* accOffset = accumulatedOffsetOf(M, gep);
                    Value &orig = getPointerOrigin(gep);
                    Value* arraySize = getPointerElemNum(orig);

                    B.SetInsertPoint(gep);
                    B.CreateCall(bounds_check_fn);

                    changed = true;
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
