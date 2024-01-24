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
        ConstantInt* constant = nullptr;

        ConstInfo(Value* hs) {
            if((this->constant = dyn_cast<ConstantInt>(hs))){
                this->isZero = constant->isZero();
                this->isOne = constant->isOne();
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
            switch (I->getOpcode())
            {
            case Instruction::Add:
                res = lhs.constant->getValue() + rhs.constant->getValue();
                break;

            case Instruction::Sub:
                res = lhs.constant->getValue() - rhs.constant->getValue();
                break;

            case Instruction::Mul:
                res = lhs.constant->getValue() * rhs.constant->getValue();
                break;

            case Instruction::AShr:
                res = lhs.constant->getValue().ashr(rhs.constant->getValue());
                break;

            case Instruction::Shl:
                res = lhs.constant->getValue().shl(rhs.constant->getValue());
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
        Value* _lhs = I->getOperand(0);
        Value* _rhs = I->getOperand(1);

        ConstInfo lhs(I->getOperand(0));
        ConstInfo rhs(I->getOperand(1));

        Instruction* ilhs;
        Instruction* irhs;

        Value* zero = ConstantInt::get(I->getType(), 0);

        switch (I->getOpcode())
        {

        case Instruction::Sub:
            ilhs = dyn_cast<Instruction>(_lhs);
            irhs = dyn_cast<Instruction>(_rhs);
            if(ilhs && irhs && ilhs->isIdenticalTo(irhs)) t->transformation = zero;
            else if(rhs.isZero) t->transformation = _lhs;
            break;

        case Instruction::Mul:
            if(rhs.isOne) t->transformation = _lhs;
            else if(lhs.isOne) t->transformation = _rhs;
            else if(rhs.isZero) t->transformation = zero;
            else if(lhs.isZero) t->transformation = zero;
            break;

        case Instruction::AShr:
        case Instruction::Shl:
        case Instruction::Add:
            if(rhs.isZero) t->transformation = _lhs;
            else if(lhs.isZero) t->transformation = _rhs;
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
            if(transformation->transformation) {
                workList.push_back(transformation);
            }
            
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
                Instruction* IU0 = dyn_cast<Instruction>(II->getOperand(0));
                unsigned index = IU0 && I->isIdenticalTo(IU0) ? 0 : 1;
                users.push_back(std::make_pair(II, index));
            }
        }

        if(!anyChange) anyChange = !users.empty();

        for (std::pair<Instruction*, unsigned> p : users){
            Instruction* II = p.first;
            II->setOperand(p.second, t->transformation);
            
            Transformation* transformation = transFunc(II);
            if(transformation->transformation) {
                workList.push_back(transformation);
            }
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