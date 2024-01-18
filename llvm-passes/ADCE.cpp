#define DEBUG_TYPE "DummyPass"
#include "utils.h"

namespace {
    class ADCE : public FunctionPass {
    public:
        static char ID;
        ADCE() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override;
    };
}

bool isTriviallyLive(Instruction &i){
    bool isVolatileLoad = false;
    Instruction *ii = &i;
    if(LoadInst* li =  dyn_cast<LoadInst>(ii)) isVolatileLoad = li->isVolatile();

    return i.mayHaveSideEffects() || i.isTerminator() || isVolatileLoad || dyn_cast<StoreInst>(ii) || dyn_cast<CallInst>(ii);
}

bool ADCE::runOnFunction(Function &F) {
    LOG_LINE("Visiting function " << F.getName());


    std::set<Instruction> liveSet;
    df_iterator_default_set<BasicBlock*> reachableBlocks;
    SmallVector<Instruction> workList;

    for(BasicBlock block : depth_first_ext(F, reachableBlocks)){
        for(Instruction &i : block.getInstList()){
            if(isTriviallyLive(i)) {
                liveSet.insert(i);
                workList.push_back(i);
            }
            else if(i.use_empty()) i.eraseFromParent();
        }
    }

    while (!workList.empty()){
        Instruction &i = workList.front();
        workList.erase(workList.begin());

        if(reachableBlocks.find(i.getParent()) != reachableBlocks.end()){
            for (Use &U : i.operands()) {
                Value *v = U.get();
                
                if(Instruction* ii = dyn_cast<Instruction>(v)){
                    if(liveSet.insert(*ii).second) workList.push_back(*ii);
                }
            }
        }
    }

    std::set<Instruction*> nonLiveInstructions;

    for(BasicBlock* bb : reachableBlocks){
        for(Instruction &i: bb->instructionsWithoutDebug()){
            Instruction *ii = &i;
            if(liveSet.find(i) == liveSet.end()) {
                ii->dropAllReferences();
                nonLiveInstructions.insert(ii);  
            }
        }
    }

    for(Instruction* i: nonLiveInstructions) i->eraseFromParent();

    return !nonLiveInstructions.empty();
}


char ADCE::ID = 1;
static RegisterPass<ADCE> X("coco-adce", "dead code elimination");