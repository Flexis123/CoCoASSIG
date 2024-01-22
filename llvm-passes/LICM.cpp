#define DEBUG_TYPE "LICM"
#include "utils.h"

namespace {
    class LICM : public LoopPass {
    public:
        static char ID;
        LICM() : LoopPass(ID) {}
        virtual bool runOnLoop(Loop *L, LPPassManager &LPM) override;
        void getAnalysisUsage(AnalysisUsage &AU) const override;
    };
}

void LICM::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
    AU.addRequired<LoopInfoWrapperPass>();
    getLoopAnalysisUsage(AU);
}


bool isLoopInvariant(Instruction *I, Loop *L){
    bool instIsRightType = I->isShift() || I->isCast() || I->isBinaryOp() 
            || dyn_cast<SelectInst>(I) || dyn_cast<GetElementPtrInst>(I);

    return instIsRightType && std::all_of(I->operands().begin(), I->operands().end(), [&](Use &U){
        Value *v = U.get();
        Instruction* II = dyn_cast<Instruction>(v);
        return dyn_cast<Constant>(v)  || (II && !L->contains(II->getParent()));
    });
}

bool isSafeToHoist(Instruction *I, Loop *L, DominatorTree *DT){
    SmallVector<BasicBlock*, 8> exitBlocks;
    L->getExitBlocks(exitBlocks);

    return !I->mayHaveSideEffects() || std::all_of(exitBlocks.begin(), exitBlocks.end(), [&](BasicBlock* BB) {return DT->dominates(I->getParent(), BB);});
}

void hoistInstructions(Loop* L, BasicBlock* BB, DominatorTree *DT, BasicBlock::iterator I, BasicBlock::iterator IE, bool &changedIr){
    for (; I != IE; ++I){
        Instruction* II = &*I; 
        if(isLoopInvariant(II, L) && isSafeToHoist(II, L, DT)){
            BasicBlock::iterator new_I = ++I;
            II->moveBefore(L->getLoopPreheader()->getTerminator());
            changedIr = true;

            return hoistInstructions(L, BB, DT, new_I, IE, changedIr);
        }
    }
}

// TODO:
// Indirectbr instructions introduce several complications. If the loop
// contains or is entered by an indirectbr instruction, it may not be possible
// to transform the loop and make these guarantees. Client code should check
// that these conditions are true before relying on them.

//Can -loop-simplify create a common preheader if there are indirect br always?
bool LICM::runOnLoop(Loop *L, LPPassManager &LPM) {
    LOG_LINE("loop: " << L->getName());
    BasicBlock *Header = L->getHeader();
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    LoopInfo *loopInfo = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    bool changedIr = false;

    for (BasicBlock *BB : L->blocks()) {
        if (DT->dominates(Header, BB) && loopInfo->getLoopFor(BB)->getLoopDepth() == L->getLoopDepth()) {
            hoistInstructions(L, BB, DT, BB->begin(), BB->end(), changedIr);
        }
    }

    return changedIr;
}


char LICM::ID = 2;
static RegisterPass<LICM> X("coco-licm", "loop invariant code motion");