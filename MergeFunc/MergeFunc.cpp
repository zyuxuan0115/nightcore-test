//===- MergeFunc.cpp - llvm pass for merging 2 serverless functions ------===//
//
// This file contains 2 components: - ChangeFuncName, MergeFunc
//
// ChangeFuncName: Change the function name of faas_func_call in callee, 
//                 otherwise the new function cannot be merged into the 
// 		   same address space due to duplication of the function 
//                 sympols.
//
// MergeFunc: Convert RPC to normal function call by changing 
//            the arguments of the function and the call 
//	      instruction.
//
//===---------------------------------------------------------------------===//
//
// This file described how to use this LLVM MergeFunc pass is in 
// https://github.com/zyuxuan0115/nightcore-test/blob/main/MergeFunc/README.md
//
//===---------------------------------------------------------------------===//

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
#include "llvm/ADT/IndexedMap.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

#define DEBUG_TYPE "MergeFunc"

namespace {
  struct MergeFunc: public ModulePass {
    static char ID; 
    MergeFunc() : ModulePass(ID) {}
    bool isRPC(Instruction* Inst);
    StringRef getRPCCalleeName(Instruction* Inst);

    bool runOnModule(Module &M) override {
      Function *CallerFunc = M.getFunction("faas_func_call");
      Function *CalleeFunc = M.getFunction("faas_func_callee"); 

      // get the RPC invocation instruction (caller instruction)
      CallInst* RPCInst;
      BasicBlock* RPCBB;
      bool hasRPCinvocation = false;
      StringRef CalleeName = "";
      for (Function::iterator BBB = CallerFunc->begin(), BBE = CallerFunc->end(); BBB != BBE; ++BBB){
        for (BasicBlock::iterator IB = BBB->begin(), IE = BBB->end(); IB != IE; IB++){
          if (isRPC(dyn_cast<Instruction>(IB))){
            hasRPCinvocation = true;
            CalleeName = getRPCCalleeName(dyn_cast<Instruction>(IB));
            RPCInst = dyn_cast<CallInst>(IB);
            RPCBB = dyn_cast<BasicBlock>(BBB);
          }
        }  
      }

      if (!hasRPCinvocation) return false;

      // based on the RPC call, create a normal function call
      // get the following arguments from the RPC Instruction
      //     - 1st argument: void* worker_handle 
      //     - 3rd argument: char* input
      //     - 4st argument: int input_length
      //     - 5st argument: char* output
      //     - 6st argument: int output_length
      std::vector<Value*> arguments;
      std::vector<Type*> argumentTypes;

      for (int i=0; i<6; i++){
        if (i==1) continue;
        arguments.push_back(RPCInst->getOperand(i));
	argumentTypes.push_back(RPCInst->getOperand(i)->getType());
      }
      ArrayRef<Type*> argTypes(argumentTypes);
      ArrayRef<Value*> args(arguments);
      FunctionType* FuncType = FunctionType::get(IntegerType::getInt32Ty(M.getContext()), argTypes, true);

      Function * NewCalleeFunc = Function::Create(FuncType, CallerFunc->getLinkage(), CalleeName, &M);
      ValueToValueMapTy VMap;
      Function::arg_iterator DestI = NewCalleeFunc->arg_begin();

      for (Function::const_arg_iterator J = CalleeFunc->arg_begin(); J != CalleeFunc->arg_end();
         ++J) {
        NewCalleeFunc->setName(J->getName());
        VMap[J] = DestI++;
      }
      SmallVector<ReturnInst*, 8> Returns;

      CloneFunctionInto(NewCalleeFunc, CalleeFunc, VMap, llvm::CloneFunctionChangeType::LocalChangesOnly, Returns);
      NewCalleeFunc->setName(CalleeName);

      // create the normal function call of the RPC 
      // and then eliminate RPC call
      // before RPC call instruction is eliminated, 
      // need to change all user instructions' operands
      // that depend on the result of the RPC call 
      CallInst* newCall = CallInst::Create(FuncType, NewCalleeFunc, args ,"", RPCInst->getNextNode());
      Value* DestRPCInst = dyn_cast<Value>(RPCInst);
      for(auto U : DestRPCInst->users()){ 
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


      // operate on the new callee (the non-RPC callee function)
      // change how RPC's callee functions returns the value.
      Value* argOutputBuf = dyn_cast<Value>(NewCalleeFunc->arg_begin()+3);
      Value* argOutputBufSize = dyn_cast<Value>(NewCalleeFunc->arg_begin()+4); 

      // get the RPC interface (context->append_output_fn) again
      CallInst* VirtualCall;
      bool hasFaasRuntimeAPI = false;
      Value* outputBuf;
      Value* outputBufSize;
      for (Function::iterator BBB = NewCalleeFunc->begin(), BBE = NewCalleeFunc->end(); BBB != BBE; ++BBB){
        for (BasicBlock::iterator IB = BBB->begin(), IE = BBB->end(); IB != IE; IB++){
          if ((isa<CallInst>(IB)) && 
              (dyn_cast<CallInst>(IB)->isIndirectCall()) && 
              (IB->getNumOperands()==4)){
            hasFaasRuntimeAPI = true;
            VirtualCall = dyn_cast<CallInst>(IB);
            outputBuf = VirtualCall->getOperand(1);
            outputBufSize = VirtualCall->getOperand(2);
          }
        }  
      }

      // replace context->append_output_fn with copying data
      // to the output buffer. Also need to erase the 
      // context->append_output_fn function call and the free()
      // function after that.
      Instruction* VirtualCallNextInst = dyn_cast<Instruction>(VirtualCall)->getNextNode();
      AllocaInst* argOutputBufSizeAddr = new AllocaInst(argOutputBufSize->getType(), 0, NULL, "", VirtualCallNextInst);
      StoreInst *storeInst1 = new StoreInst(argOutputBufSize, argOutputBufSizeAddr, VirtualCallNextInst);
      LoadInst *loadInst1 = new LoadInst (argOutputBufSize->getType(), argOutputBufSizeAddr, "", VirtualCallNextInst);
      StoreInst *storeInst2 = new StoreInst(outputBufSize, dyn_cast<Value>(loadInst1), VirtualCallNextInst);

      AllocaInst* argOutputBufAddr = new AllocaInst(argOutputBuf->getType(), 0, NULL, "", VirtualCallNextInst);
      StoreInst *storeInst3 = new StoreInst(argOutputBuf, argOutputBufAddr, VirtualCallNextInst); 
      LoadInst *loadInst2 = new LoadInst(argOutputBuf->getType(), argOutputBufAddr, "", VirtualCallNextInst);
      StoreInst *storeInst4 = new StoreInst(outputBuf, dyn_cast<Value>(loadInst2), VirtualCallNextInst);

      // remove the free() function call after RPC returns
      Instruction* nextInst = VirtualCallNextInst;
      std::vector<Instruction*> freeCalls;
      while (nextInst){
        if (isa<CallInst>(nextInst)){
          CallInst* call = dyn_cast<CallInst>(nextInst);
          Function* func = call->getCalledFunction();
          if ((func) && (func->getName()=="free")){
            freeCalls.push_back(nextInst);
          }
        }
        nextInst = nextInst->getNextNode();
      }
      for (auto inst: freeCalls)
        inst->eraseFromParent();

      VirtualCall->eraseFromParent();

      return false;
    }
  };

  bool MergeFunc::isRPC(Instruction* Inst){
    // test if it's a virtual function call
    // if it is, test if it's a RPC invocation 
    // (7 arguments & the 2nd argument is const char*)
    StringRef CalleeName = "";
    bool hasRPCinvocation = false;
    if ((isa<CallInst>(Inst)) && (dyn_cast<CallInst>(Inst)->isIndirectCall())){
      CallInst* VirtualCall = dyn_cast<CallInst>(Inst);
      if (VirtualCall->getNumOperands()==7){
        Value* operand = VirtualCall->getOperand(1);
        if (isa<ConstantExpr>(operand)){
          Value *firstop = dyn_cast<ConstantExpr>(operand)->getOperand(0);
          if (isa<GlobalVariable>(firstop)){
            GlobalVariable* GV = dyn_cast<GlobalVariable>(firstop);
	    ConstantDataArray* CDA = dyn_cast<ConstantDataArray>(GV->getInitializer());
            CalleeName = CDA->getAsCString();
            if (CalleeName.str() != ""){
              hasRPCinvocation = true;
	    }
	  }
	}
      }
    }
    return hasRPCinvocation;
  }

  StringRef MergeFunc::getRPCCalleeName(Instruction* Inst){
    StringRef CalleeName = "";
    if ((isa<CallInst>(Inst)) && (dyn_cast<CallInst>(Inst)->isIndirectCall())){
      CallInst* VirtualCall = dyn_cast<CallInst>(Inst);
      if (VirtualCall->getNumOperands()==7){
        Value* operand = VirtualCall->getOperand(1);
        if (isa<ConstantExpr>(operand)){
          Value *firstop = dyn_cast<ConstantExpr>(operand)->getOperand(0);
          if (isa<GlobalVariable>(firstop)){
            GlobalVariable* GV = dyn_cast<GlobalVariable>(firstop);
	    ConstantDataArray* CDA = dyn_cast<ConstantDataArray>(GV->getInitializer());
            CalleeName = CDA->getAsCString();
	  }
	}
      }
    }
    return CalleeName;
  }
}

char MergeFunc::ID = 0;
static RegisterPass<MergeFunc> X("MergeFunc", "Convert RPC to normal function call by changing the arguments of the function and the call instruction");



namespace {
  struct ChangeFuncName : public ModulePass {
    static char ID; 
    ChangeFuncName() : ModulePass(ID) {}
  
    bool runOnModule(Module &M) override {
      std::vector<Function*> toBeRemoved;
      for (auto F = M.begin();F!=M.end() ;F++){
      	if (F->getName()=="faas_func_call") {
           Function* func = dyn_cast<Function>(F);
      	   func->setName("faas_func_callee");
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

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };
}

char ChangeFuncName::ID = 0;
static RegisterPass<ChangeFuncName>
Y("ChangeFuncName", "Change the function name of faas_func_call in callee, otherwise the new function cannot be merged into the same address space due to duplicate of the function sympols");

