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
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

#define DEBUG_TYPE "MergeFunc"

STATISTIC(MergeFuncCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct MergeFunc : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    MergeFunc() : ModulePass(ID) {}

    bool runOnModule(Module &M) override {
      Function *CallerFunc = M.getFunction("faas_func_call");
      Function *CalleeFunc = M.getFunction("faas_func_call_Bar"); 

      //  get the RPC invocation instruction (caller instruction)
      CallInst* RPCInst;
      BasicBlock* RPCBB;
      bool hasRPCinvocation = false;
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
                    hasRPCinvocation = true;
		    errs()<<"@@@ caller name = "<<CalleeName.str()<<"\n";
		  }
		}
	      }
	    }
          }
        }  
      }

      if (!hasRPCinvocation) return false;

      // based on the RPC call, create a normal function call
      // get the following arguments from the RPC
      // 3rd argument: char* input
      // 4st argument: int input_length
      // 5st argument: char* output
      // 6st argument: int output_length
      std::vector<Value*> arguments;
      std::vector<Type*> argumentTypes;
      for (int i=2; i<6; i++){
	llvm::errs()<<*(RPCInst->getOperand(i))<<"\n";
        arguments.push_back(RPCInst->getOperand(i));
	argumentTypes.push_back(RPCInst->getOperand(i)->getType());
      }
      ArrayRef<Type*> argTypes(argumentTypes);
      ArrayRef<Value*> args(arguments);
      FunctionType* FuncType = FunctionType::get(IntegerType::getInt32Ty(M.getContext()), argTypes, true);

      Function * NewCalleeFunc = Function::Create(FuncType, CallerFunc->getLinkage(), CalleeName, &M);
      ValueToValueMapTy VMap;
      Function::arg_iterator DestI = NewCalleeFunc->arg_begin();

      Function::arg_iterator OrigI = CalleeFunc->arg_begin();
      for (Function::const_arg_iterator J = CalleeFunc->arg_begin(); J != CalleeFunc->arg_begin()+1;
         ++J) {
        NewCalleeFunc->setName(J->getName());
        VMap[J] = NewCalleeFunc->arg_begin();
      } 

      for (Function::const_arg_iterator J = CalleeFunc->arg_begin()+1; J != CalleeFunc->arg_end();
         ++J) {
        NewCalleeFunc->setName(J->getName());
        VMap[J] = DestI++;
      }
      SmallVector<ReturnInst*, 8> Returns;

      CloneFunctionInto(NewCalleeFunc, CalleeFunc, VMap, llvm::CloneFunctionChangeType::LocalChangesOnly, Returns);
      NewCalleeFunc->setName(CalleeName);

      // create the normal function call of the RPC 
      // and then elilinate RPC
      //FunctionCallee c(NewCalleeFunc);
      //CallInst* newCall = CallInst::Create(c, args, "", RPCInst->getNextNode());

      CallInst* newCall = CallInst::Create(FuncType, NewCalleeFunc, args ,"", RPCInst->getNextNode());
      // !!! before remove this line, we need to change the source register of 
      // the user of this instruction.
      Value* DestRPCInst = dyn_cast<Value>(RPCInst);
      for(auto U : DestRPCInst->users()){  // U is of type User*
        for (auto op = U->op_begin(); op != U->op_end(); op++){
          Value* op_value = dyn_cast<Value>(op);
          if (op_value == DestRPCInst){
            Value* DestNewCall = dyn_cast<Value>(newCall);
            *op = DestNewCall;
          }
        }
      }
     
      RPCInst->eraseFromParent();
      // remove old callee function (RPC version)
      CalleeFunc->eraseFromParent();
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
