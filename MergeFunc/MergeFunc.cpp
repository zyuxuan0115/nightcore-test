//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/DataLayout.h" //#include "llvm/Target/TargetData.h"
#include "llvm/ADT/IndexedMap.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Mangler.h"
using namespace llvm;

#define DEBUG_TYPE "MergeFunc"

STATISTIC(MergeFuncCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct MergeFunc : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    MergeFunc() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      LLVMContext &ctx = F.getContext();
      if (F.getName()=="faas_func_call") {
        for (auto arg = F.arg_begin(); arg != F.arg_end(); arg++){
          errs()<<"args!\n";  
        }	
        for (Function::iterator BBB = F.begin(), BBE = F.end(); BBB != BBE; ++BBB){// for all instructions in a block
          for (BasicBlock::iterator IB = BBB->begin(), IE = BBB->end(); IB != IE; IB++){
            Instruction* I = dyn_cast<Instruction>(IB);
	    llvm::errs()<<*I<<"\n";
	    // virtual function
            if ((isa<CallInst>(IB)) && (dyn_cast<CallInst>(IB)->isIndirectCall())){
              CallInst* VirtualCall = dyn_cast<CallInst>(IB);
	      if (VirtualCall->getNumOperands()==7){
              llvm::errs()<<"@@@ the indirect call instruction:"<<*VirtualCall<<"\n";
	      //Value* v=call->getCalledValue();
              //Value* sv = v->stripPointerCasts();
	      //Instruction* Instr = dyn_cast<Instruction>(v);
	      //Instr->getNumOperands();
	        llvm::StringRef CallerName = "";
	        for (unsigned i=0; i<VirtualCall->getNumOperands(); i++){
		  Value* operand = VirtualCall->getOperand(i);
		  if (isa<ConstantExpr>(operand)){
		    Value *firstop = dyn_cast<ConstantExpr>(operand)->getOperand(0);
		    if (isa<GlobalVariable>(firstop)){
                      GlobalVariable* GV = dyn_cast<GlobalVariable>(firstop);
		      ConstantDataArray* CDA = dyn_cast<ConstantDataArray>(GV->getInitializer());
		      if (i==1){
		        CallerName = CDA->getAsCString();
		      }
		    }
		  }
	        }
		if (CallerName != ""){
                  // it's a RPC
		  Instruction* NextInst = VirtualCall->getNextNode();
		}
	      }
	      Function* callFunc = dyn_cast<CallInst>(IB)->getCalledFunction();
	    }
          }
        }	  
      }
      return false;
    }
  };
}

char MergeFunc::ID = 0;
static RegisterPass<MergeFunc> X("MergeFunc", "Merge Function Pass");

namespace {
  // ChangeFuncName - The second implementation with getAnalysisUsage implemented.
  struct ChangeFuncName : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    ChangeFuncName() : ModulePass(ID) {}
  
    bool runOnModule(Module &M) override {
      std::vector<Function*> toBeRemoved;
      for (auto F = M.begin();F!=M.end() ;F++){
      	if (F->getName()=="faas_func_call") {
           Function* func = dyn_cast<Function>(F);
      	   func->setName("faas_func_call_Bar");
        }
	else if ((F->getName()=="faas_init") ||
	(F->getName()=="faas_destroy_func_worker") || 
	(F->getName()=="faas_create_func_worker")){
	   Function* func = dyn_cast<Function>(F);
	   toBeRemoved.push_back(func);
	}
      }
      while(!toBeRemoved.empty()){
        toBeRemoved.back()->eraseFromParent();
	toBeRemoved.pop_back();
      }
      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };
}

char ChangeFuncName::ID = 0;
static RegisterPass<ChangeFuncName>
Y("ChangeFuncName", "Hello World Pass (with getAnalysisUsage implemented)");
