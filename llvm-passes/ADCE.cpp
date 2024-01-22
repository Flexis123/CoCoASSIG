#define DEBUG_TYPE "ADCE"
#include "utils.h"

//are they gonna test if(false)/while(false) ?

namespace {
    class ADCE : public FunctionPass {
    public:
        static char ID;
        ADCE() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override;
    };
}

bool isTriviallyLive(Instruction* i){
    bool isVolatileLoad = false;
    if(LoadInst* li = dyn_cast<LoadInst>(i)) isVolatileLoad = li->isVolatile();

    return i->mayHaveSideEffects() || i->isTerminator() || isVolatileLoad || dyn_cast<StoreInst>(i) || dyn_cast<CallInst>(i);
}

void basicBlockTrivialAnalysis(BasicBlock::iterator I, BasicBlock::iterator IE, bool &changedIr, std::set<Instruction*> &liveSet, SmallVector<Instruction*,  128> &workList){
    for (; I != IE; ++I){
        Instruction* ii = &*I; 

        if(isTriviallyLive(ii)) {
            liveSet.insert(ii);
            workList.push_back(ii);
        }
        else if(ii->use_empty()) {
            changedIr = true;
            return basicBlockTrivialAnalysis(ii->eraseFromParent(), IE, changedIr, liveSet, workList);
        }
    }
}



bool ADCE::runOnFunction(Function &F) {
    std::set<Instruction*> liveSet;
    df_iterator_default_set<BasicBlock*, 32> reachableBlocks;
    SmallVector<Instruction*,  128> workList;
    bool changedIr = false;
    
    for(BasicBlock* block : depth_first_ext(&F, reachableBlocks)) 
        basicBlockTrivialAnalysis(block->begin(), block->end(), changedIr, liveSet, workList);
        
    
    while (!workList.empty()){
        Instruction* i = workList.front();
        workList.erase(workList.begin());

        if(reachableBlocks.find(i->getParent()) != reachableBlocks.end()){
            for (Use &U : i->operands()) {
                Value *v = U.get();
                
                if(Instruction* ii = dyn_cast<Instruction>(v)){
                    if(liveSet.insert(ii).second) workList.push_back(ii);  
                }
            }
        }
    }

    std::set<Instruction*> nonLiveInstructions;

    for(BasicBlock* bb : reachableBlocks){
        for(Instruction &i: *bb){
            Instruction *ii = &i;
            if(liveSet.find(ii) == liveSet.end()) {
                ii->dropAllReferences();
                nonLiveInstructions.insert(ii);  
            } 
        }
    }

    for(Instruction* i: nonLiveInstructions) i->eraseFromParent();

    return changedIr || !nonLiveInstructions.empty();
}


char ADCE::ID = 1;
static RegisterPass<ADCE> X("coco-adce", "dead code elimination");