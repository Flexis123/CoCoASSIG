#define DEBUG_TYPE "ConstProp"
#include "utils.h"

namespace {
    class ConstProp : public FunctionPass {
    public:
        static char ID;
        ConstProp() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override;
    };
}

bool isTriviallyConstant(Instruction &I){
    bool isRightInst = I.isArithmeticShift() || I.getOpcode() == Instruction::Add;
    isRightInst |= I.getOpcode() == Instruction::Sub || I.getOpcode() == Instruction::Mul;

    return isRightInst && std::all_of(I.operands().begin(), I.operands().end(), [&](Use &U){
        return dyn_cast<ConstantInt>(U.get());
    });
}

bool ConstProp::runOnFunction(Function &F) {
    //add trivially constant instructions to worklist
    df_iterator_default_set<BasicBlock*, 32> reachableBlocks;
    SmallVector<Instruction*,  128> workList;
    bool changedIr = false;
    
    for(BasicBlock* BB : depth_first_ext(&F, reachableBlocks)){
        for(Instruction &I: *BB) if(isTriviallyConstant(I)) workList.push_back(&I);
    }
    changedIr = !workList.empty();

    //propgate constants
    while(!workList.empty()){
        Instruction* I = workList.front();
        workList.erase(workList.begin());

        //todo calculate constant val;
        // take in account type of int
        ConstantInt &lhs = *dyn_cast<ConstantInt>(I->getOperand(0));
        ConstantInt &rhs = *dyn_cast<ConstantInt>(I->getOperand(1));
        ConstantInt *res;

        switch (I->getOpcode())
        {
        case Instruction::Add:
            /* code */
            break;

        case Instruction::Sub:
            /* code */
            break;

        case Instruction::Mul:
            /* code */
            break;
        
        default:
            break;
        }

        LOG_LINE("inst" << *I << "isBinaryOp: " << I->isBinaryOp());
       // for (User *U : I->users()) {
            //int index = I->isIdenticalTo(dyn_cast<Instruction>(U->getOperand(0))) ? 0 : 1;

            //replace with constant val operand and index
            //and check after if both operands are Constant if constant add to work list
        //}
    }

    return changedIr;
}


char ConstProp::ID = 0;
static RegisterPass<ConstProp> X("coco-constprop", "Example LLVM pass printing each function it visits, and every call instruction it finds");