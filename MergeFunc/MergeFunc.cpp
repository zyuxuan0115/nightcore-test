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
      if (F.getName()=="faas_func_call") {
        for (auto arg = F.arg_begin(); arg != F.arg_end(); arg++){
          errs()<<"args!\n";  
        }	
        for (Function::iterator BBB = F.begin(), BBE = F.end(); BBB != BBE; ++BBB){// for all instructions in a block
          for (BasicBlock::iterator IB = BBB->begin(), IE = BBB->end(); IB != IE; IB++){
            Instruction* I = dyn_cast<Instruction>(IB);
	    llvm::errs()<<*I<<"\n";
            if ((isa<CallInst>(IB)) && (dyn_cast<CallInst>(IB)->isIndirectCall())){
      		  //if (isa<CallInst>(IB) && ( (dyn_cast<CallInst>(IB)->getCalledFunction() == NULL) || 
	//					  (dyn_cast<CallInst>(IB)->getCalledFunction()->isDeclaration()))){
              CallInst* call = dyn_cast<CallInst>(IB);
              llvm::errs()<<"@@@ "<<*call<<"is indirect call\n";
	      //Value* v=call->getCalledValue();
              //Value* sv = v->stripPointerCasts();
	      //Instruction* Instr = dyn_cast<Instruction>(v);
	      //Instr->getNumOperands();
	      errs()<<"@@@ number of operands: "<<call->getNumOperands()<<"\n";
	      for (unsigned i=0; i<call->getNumOperands(); i++){
		Value* operand = call->getOperand(i);
		if (isa<ConstantExpr>(operand)){
		  Value *firstop = dyn_cast<ConstantExpr>(operand)->getOperand(0);
		  if (isa<GlobalVariable>(firstop)){
                    GlobalVariable* GV = dyn_cast<GlobalVariable>(firstop);
		    ConstantDataArray* CDA = dyn_cast<ConstantDataArray>(GV->getInitializer());
		    llvm::StringRef real_string = CDA->getAsCString();
		    errs()<<"### "<<real_string.str()<<"\n";
		  }
		}
	      }
	      Function* callFunc = dyn_cast<CallInst>(IB)->getCalledFunction();
	    }
          }
        }	  
        ++MergeFuncCounter;
        errs() << "MergeFunc: ";
        errs().write_escaped(F.getName()) << '\n';
      }
      return false;
    }
  };
}

char MergeFunc::ID = 0;
static RegisterPass<MergeFunc> X("MergeFunc", "Merge Function Pass");

namespace {
  // Hello2 - The second implementation with getAnalysisUsage implemented.
  struct MergeFunc2 : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    MergeFunc2() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
     
      ++MergeFuncCounter;
      errs() << "MergeFunc: ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };
}

char MergeFunc2::ID = 0;
static RegisterPass<MergeFunc2>
Y("MergeFunc2", "Hello World Pass (with getAnalysisUsage implemented)");
