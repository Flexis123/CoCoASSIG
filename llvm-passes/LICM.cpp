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

bool LICM::runOnLoop(Loop *L, LPPassManager &LPM) {
    BasicBlock *Header = L->getHeader();
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    LoopInfo *loopInfo = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    bool changedIr = false;


    for (BasicBlock *BB : L->blocks()) {
        if (DT->dominates(Header, BB) && loopInfo->getLoopFor(BB)->getLoopDepth() == L->getLoopDepth()) {
            SmallVector<Instruction*,  128> workList;

            for(Instruction &I: *BB){
                Instruction* II = &I;
                if(isLoopInvariant(II, L)) workList.push_back(II);
            }

            while(!workList.empty()){
                Instruction* II = workList.front();
                workList.erase(workList.begin());

                if(isSafeToHoist(II, L, DT)){
                    II->moveBefore(L->getLoopPreheader()->getTerminator());

                    for(User *U: II->users()){
                        Instruction *III;
                        if ((III = dyn_cast<Instruction>(U)) && isLoopInvariant(III, L)) workList.push_back(III);
                    }
                }
            }
        }
    }

    return changedIr;
}


char LICM::ID = 2;
static RegisterPass<LICM> X("coco-licm", "loop invariant code motion");