#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/Analysis/MemoryLocation.h"

using namespace llvm;
using namespace std;

namespace {
    struct CustomCallGraphExtractionPass : public ModulePass {
        static char ID;

        CustomCallGraphExtractionPass() : ModulePass(ID) {}


        bool runOnModule(Module &M) {
        	vector<string> chain;
        	for(auto &F : M) {
        		modifyBasicBlocks(F);
        	}

        	Function * mainFunction = M.getFunction("main");
        	constructCallChain(*mainFunction, chain);

        	M.dump();

        	errs() << "Chain:\n";
        	string output = "";
            for (const auto& functionName : chain) {
                output += functionName + "->";
            }
            output = output.substr(0, output.size() - 2);
        	errs() << output << "\n";

            return true;
        }

        void modifyBasicBlocks(Function &F) {
        	StringRef functionName = F.getName();
        	std::vector<CallInst *> callInsts;
        	for(auto &BB : F) {
                for(auto &I : BB) {
                    if(CallInst * callInst = dyn_cast<CallInst>(&I)) {
                        callInsts.push_back(callInst);
                    }
                }
            }

            //split basic blocks
            for(CallInst * callInst : callInsts) {
            	BasicBlock &BB = *(callInst->getParent());
            	StringRef calledFunctionName = callInst->getCalledFunction()->getName();
                BasicBlock* newBlock = BB.splitBasicBlock(callInst, *(new Twine(calledFunctionName + "^")));
                BasicBlock* epliogue = newBlock->splitBasicBlock(callInst->getNextNode());
            }

            //name basic blocks
            int bbCounter = 0;
            for(auto &BB : F) {
            	StringRef basicBlockName = BB.getName();
            	string terminator = "^";
            	bool isCallBlock = basicBlockName.data()[basicBlockName.size() - 1] == '^';
            	if(!isCallBlock) {
        			bbCounter++;
        			BB.setName(functionName + to_string(bbCounter));
            	}
            }
        }


        void constructCallChain(Function &F, vector<string> &chain) {
        	bool containsCalls = false;
        	for(auto &BB : F) {
    			StringRef basicBlockName = BB.getName();
            	bool isCallBlock = basicBlockName.data()[basicBlockName.size() - 1] == '^';
                if(!isCallBlock) {
                    chain.push_back(basicBlockName);
                }
            	else {
            		BasicBlock::iterator I = BB.begin();
            		CallInst * callInst = dyn_cast<CallInst>(I);
            		BB.setName(basicBlockName.substr(0, basicBlockName.size() - 1)); //remove suffix
            		constructCallChain(*(callInst->getCalledFunction()), chain);
            	}
        	}
        }
    };
}

char CustomCallGraphExtractionPass::ID = 0;

static RegisterPass<CustomCallGraphExtractionPass> X("cg_extract", "Call graph extractor pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
