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

bool BoundsChecker::runOnModule(Module &M) {
   
    for (Function &F : M) {
        if (!shouldInstrument(&F))
            continue;

        GetElementPtrInst *gep;
        for(BasicBlock &BB : F){
            for(Instruction &I: BB){
                if((gep = dyn_cast<GetElementPtrInst>(&I)) && gep->getNumOperands() < 3){
                    if(Value* V = accumulatedOffsetOf(M, gep)){
                        LOG_LINE("acc ooffset of " << I << " is " << *V);
                    }
                }
            }
        }
    }

    return false;
}

// Register the pass with LLVM so we can invoke it with opt. The first argument
// to RegisterPass is the commandline switch to run this pass (e.g., opt
// -coco-BoundsChecker, the second argument is a description shown in the help
// text about this pass.
char BoundsChecker::ID = 4;
static RegisterPass<BoundsChecker> X("coco-boundschecker", "Bound check instrumentation");
