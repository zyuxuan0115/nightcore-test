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
  struct MergeFunc : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    MergeFunc() : ModulePass(ID) {}

    bool runOnModule(Module &M) override {
      Function *CallerFunc, *CalleeFunc; 
      for (auto F = M.begin(); F!=M.end(); F++){
        if (F->getName()=="faas_func_call")  CallerFunc = dyn_cast<Function>(F);
        else if (F->getName()=="faas_func_call_Bar") CalleeFunc = dyn_cast<Function>(F);
      }

      //  get the RPC invocation instruction (caller instruction)
      CallInst* RPCInst;
      BasicBlock* RPCBB;
      llvm::StringRef CalleeName = "";
      for (Function::iterator BBB = CallerFunc->begin(), BBE = CallerFunc->end(); BBB != BBE; ++BBB){
        for (BasicBlock::iterator IB = BBB->begin(), IE = BBB->end(); IB != IE; IB++){
	  // test if it's a virtual function call
	  // if it is, test if it's a RPC invocation 
	  // (7 arguments & the 2nd argument is const char*)
          if ((isa<CallInst>(IB)) && (dyn_cast<CallInst>(IB)->isIndirectCall())){
            CallInst* VirtualCall = dyn_cast<CallInst>(IB);
	    if (VirtualCall->getNumOperands()==7){
              llvm::errs()<<"@@@ the indirect call instruction:"<<*VirtualCall<<"\n";
	      //Value* v=call->getCalledValue();
              //Value* sv = v->stripPointerCasts();
              Value* operand = VirtualCall->getOperand(1);
              if (isa<ConstantExpr>(operand)){
		Value *firstop = dyn_cast<ConstantExpr>(operand)->getOperand(0);
		if (isa<GlobalVariable>(firstop)){
                  GlobalVariable* GV = dyn_cast<GlobalVariable>(firstop);
		  ConstantDataArray* CDA = dyn_cast<ConstantDataArray>(GV->getInitializer());
		  CalleeName = CDA->getAsCString();
		  if (CalleeName.str() != ""){
		    RPCInst = VirtualCall;
		    RPCBB = dyn_cast<BasicBlock>(IB);
		    errs()<<"@@@ caller name = "<<CalleeName.str()<<"\n";
		  }
		}
	      }
	    }
          }
        }  
      }

      // based on the RPC call, create a normal function call
      // get the following arguments from the RPC
      // 3rd argument: char* input
      // 4st argument: int input_length
      // 5st argument: char* output
      // 6st argument: int output_length
      std::vector<Value*> arguments;
      std::vector<Type*> argumentTypes;
      for (int i=3; i<7; i++){
        arguments.push_back(RPCInst->getOperand(i));
	argumentTypes.push_back(RPCInst->getOperand(i)->getType());
      }
      ArrayRef<Type*> argTypes(argumentTypes);
      ArrayRef<Value*> args(arguments);

      FunctionType* FuncType = FunctionType::get(IntegerType::getInt32Ty(M.getContext()), argTypes, true);
      FunctionCallee c = M.getOrInsertFunction("Bar", FuncType);

      CallInst* newcall = CallInst::Create(c, args, "", RPCInst->getNextNode());
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
