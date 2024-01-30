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

class ConstInfo{
    public:
        bool isZero = false;
        bool isOne = false;
        bool isAllOnes = false;
        bool isAllZeros = false;
        ConstantInt* constant = nullptr;

        ConstInfo(Value* hs) {
            if((this->constant = dyn_cast<ConstantInt>(hs))){
                this->isZero = constant->isZero();
                this->isOne = constant->isOne();
                this->isAllOnes = constant->isAllOnesValue();
            }
        }
};

class Transformation{
    public:
        Value* transformation = nullptr;
        Instruction* I = nullptr;
};


Transformation* getConstPropagationTransformation(Instruction *I){
    Transformation* t = new Transformation();
    t->I = I;

    if(I->isBinaryOp()){
        ConstInfo lhs(I->getOperand(0));
        ConstInfo rhs(I->getOperand(1));

        if(lhs.constant && rhs.constant){
            APInt res;
            APInt clhs = lhs.constant->getValue();
            APInt crhs = rhs.constant->getValue();

            switch (I->getOpcode())
            {
            case Instruction::Add:
                res = clhs + crhs;
                break;

            case Instruction::Sub:
                res = clhs - crhs;
                break;

            case Instruction::Mul:
                res = clhs * crhs;
                break;

            case Instruction::AShr:
                res = clhs.ashr(crhs);
                break;

            case Instruction::LShr:
                res = clhs.lshr(crhs);
                break;

            case Instruction::Shl:
                res = clhs.shl(crhs);
                break;

            case Instruction::URem:
                res = clhs.urem(crhs);
                break;

            case Instruction::SRem:
                res = clhs.srem(crhs);
                break;

            case Instruction::UDiv:
                res = clhs.udiv(crhs);
                break;

            case Instruction::SDiv:
                res = clhs.sdiv(crhs);
                break;

            case Instruction::And:
                res = clhs & crhs;
                break;

            case Instruction::Or:
                res = clhs | crhs;
                break;

            case Instruction::Xor:
                res = clhs ^ crhs;
                break;
            
            default:
                break;
            }

            t->transformation = ConstantInt::get(I->getContext(), res);
        }
    }
    return t;
}

Transformation* getConstFoldTransformation(Instruction *I){
    Transformation* t = new Transformation();
    t->I = I;

    if(I->isBinaryOp()){
        //BinaryOperator* op;
        Value* _lhs = I->getOperand(0);
        Value* _rhs = I->getOperand(1);

        ConstInfo lhs(I->getOperand(0));
        ConstInfo rhs(I->getOperand(1));

        Instruction* ilhs = dyn_cast<Instruction>(_lhs);
        Instruction* irhs = dyn_cast<Instruction>(_lhs);

        Value* zero = ConstantInt::get(I->getType(), 0);

        switch (I->getOpcode())
        {

        case Instruction::Sub:
            if(ilhs && irhs && ilhs->isIdenticalTo(irhs)) t->transformation = zero;
            else if(rhs.isZero) t->transformation = _lhs;
            break;

        case Instruction::Mul:
            if(rhs.isZero) t->transformation = _rhs;
            else if(lhs.isZero) t->transformation = _lhs;
            else if(rhs.isOne) t->transformation = _lhs;
            else if(lhs.isOne) t->transformation = _rhs;
            break;

        case Instruction::AShr:
        case Instruction::Shl:
        case Instruction::Add:
            if(rhs.isZero) t->transformation = _lhs;
            else if(lhs.isZero) t->transformation = _rhs;
            break;

        case Instruction::URem:
        case Instruction::SRem:
        case Instruction::UDiv:
        case Instruction::SDiv:
            if(lhs.isZero) t->transformation = _lhs;
            else if(rhs.isOne) t->transformation = _lhs;
            break;

        case Instruction::Or:
            if(ilhs && irhs && ilhs->isIdenticalTo(irhs)) t->transformation = _lhs;
            else if(rhs.isZero) t->transformation = _lhs;
            else if(lhs.isZero) t->transformation = _rhs;
            else if(lhs.isAllOnes && irhs && !irhs->mayHaveSideEffects()) t->transformation = _lhs;
            else if(rhs.isAllOnes && ilhs && !ilhs->mayHaveSideEffects()) t->transformation = _rhs;
            break;

        case Instruction::And:
            if(ilhs && irhs && ilhs->isIdenticalTo(irhs)) t->transformation = _lhs;
            else if(rhs.isZero && ilhs && !ilhs->mayHaveSideEffects()) t->transformation = _rhs;
            else if(lhs.isZero && irhs && !irhs->mayHaveSideEffects()) t->transformation = _lhs;
            else if(rhs.isAllOnes) t->transformation = _lhs;
            else if(lhs.isAllOnes) t->transformation = _rhs;
            break;

        case Instruction::Xor:
            if(ilhs && irhs && ilhs->isIdenticalTo(irhs) && !ilhs->mayHaveSideEffects() && !irhs->mayHaveSideEffects()) t->transformation = zero;
            break;
        
        default:
            break;
        }
    }
    return t;
}

bool transformationPass(Function &F, Transformation*(*transFunc)(Instruction*)){
    df_iterator_default_set<BasicBlock*, 32> reachableBlocks;
    SmallVector<Transformation*,  128> workList;
    bool anyChange = false;

    for(BasicBlock* BB : depth_first_ext(&F, reachableBlocks)){
        for(Instruction &I: *BB) {
            Transformation* transformation = transFunc(&I);
            if(transformation->transformation) workList.push_back(transformation);
        }
    }

    //propgate constants
    while(!workList.empty()){
        Transformation* t = workList.front();
        workList.erase(workList.begin());
        Instruction* I = t->I;

        SmallVector<std::pair<Instruction*, unsigned>, 32> users;
        for (User *U : I->users()) {
            if(Instruction* II = dyn_cast<Instruction>(U)){
                unsigned i = 0;
                
                for(Use &U: II->operands()){
                    Instruction* IU = dyn_cast<Instruction>(U);
                    if(IU && I->isIdenticalTo(IU)) break;
                    i++;
                }

                users.push_back(std::make_pair(II, i));
            }
        }

        if(!anyChange) anyChange = !users.empty();

        for (std::pair<Instruction*, unsigned> p : users){
            Instruction* II = p.first;
            II->setOperand(p.second, t->transformation);
            
            Transformation* transformation = transFunc(II);
            if(transformation->transformation) workList.push_back(transformation);
        }

        delete t;
    }

    return anyChange;
}

bool ConstProp::runOnFunction(Function &F) {
    bool changedIr = false;

    while(transformationPass(F, getConstPropagationTransformation) || transformationPass(F, getConstFoldTransformation)){
        changedIr = true;
    }

    return changedIr;
}


char ConstProp::ID = 0;
static RegisterPass<ConstProp> X("coco-constprop", "Example LLVM pass printing each function it visits, and every call instruction it finds");