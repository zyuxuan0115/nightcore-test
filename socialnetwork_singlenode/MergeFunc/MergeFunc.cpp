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
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Analysis/CallGraph.h"
#include <fstream>
#include <unordered_set>
using namespace llvm;

#define DEBUG_TYPE "MergeFunc"

namespace {
  struct MergeFunc: public ModulePass {
    static char ID; 
    MergeFunc() : ModulePass(ID) {}
    bool isRPC(Instruction* Inst);
    StringRef getRPCCalleeName(Instruction* Inst);

    bool runOnModule(Module &M) override {
      Function *CallerFunc = M.getFunction("_ZN10FaasWorker15ClientTransport5flushEv");
      Function *CalleeFunc = M.getFunction("faas_func_call_callee"); 

      // get the RPC invocation instruction (caller instruction)
      CallInst* RPCInst;
      BasicBlock* RPCBB;
      bool hasRPCinvocation = false;
      StringRef CalleeName = "";
      for (Function::iterator BBB = CallerFunc->begin(), BBE = CallerFunc->end(); BBB != BBE; ++BBB){
        for (BasicBlock::iterator IB = BBB->begin(), IE = BBB->end(); IB != IE; IB++){
          if (isRPC(dyn_cast<Instruction>(IB))){
            hasRPCinvocation = true;
        //      CalleeName = getRPCCalleeName(dyn_cast<Instruction>(IB));
            RPCInst = dyn_cast<CallInst>(IB);
            RPCBB = dyn_cast<BasicBlock>(BBB);
          }
        }  
      }


      if (!hasRPCinvocation) return false;

      errs()<<"### "<<*RPCInst<<"\n";

      // based on the RPC call, create a normal function call
      // get the following arguments from the RPC Instruction
      //     - 1st argument: void* worker_handle 
      //     - 3rd argument: char* input
      //     - 4th argument: int input_length
      //     - 5th argument: char* output
      //     - 6th argument: int output_length
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

      Function * NewCalleeFunc = Function::Create(FuncType, CallerFunc->getLinkage(), "ComposePostService_new", &M);
      ValueToValueMapTy VMap;
      Function::arg_iterator DestI = NewCalleeFunc->arg_begin();

      for (Function::const_arg_iterator J = CalleeFunc->arg_begin(); J != CalleeFunc->arg_end();
         ++J) {
        NewCalleeFunc->setName(J->getName());
        VMap[J] = DestI++;
      }
      SmallVector<ReturnInst*, 8> Returns;

      CloneFunctionInto(NewCalleeFunc, CalleeFunc, VMap, llvm::CloneFunctionChangeType::LocalChangesOnly, Returns);
      NewCalleeFunc->setName("ComposePostService_new");

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

/*       
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
*/
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
//        Value* operand = VirtualCall->getOperand(1);
//        if (isa<ConstantExpr>(operand)){
//          Value *firstop = dyn_cast<ConstantExpr>(operand)->getOperand(0);
//          if (isa<GlobalVariable>(firstop)){
//            GlobalVariable* GV = dyn_cast<GlobalVariable>(firstop);
//	    ConstantDataArray* CDA = dyn_cast<ConstantDataArray>(GV->getInitializer());
//            CalleeName = CDA->getAsCString();
//            if (CalleeName.str() != ""){
//              hasRPCinvocation = true;
//	    }
//	  }
//	}
	errs()<<"@@@ find a virtual func call that has 7 args\n";
        hasRPCinvocation = true;
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
  static cl::opt<std::string> writeFuncSymbol("write-func-symbol", 
                                            cl::desc("write all function symbols of the current function (normally caller function) to a file"), 
                                            cl::value_desc("filename"));
  static cl::opt<std::string> readFuncSymbol("read-func-symbol", 
                                            cl::desc("read all function symbols from a file"), 
                                            cl::value_desc("filename"));
  struct ChangeFuncNames : public ModulePass {
    static char ID; 
    ChangeFuncNames() : ModulePass(ID) {}
  
    bool runOnModule(Module &M) override {
      if (!writeFuncSymbol.empty()){
        std::ofstream outputFile;
        outputFile.open (writeFuncSymbol);
        for (auto F = M.begin(); F!=M.end(); F++){
          outputFile << F->getName().str() <<"\n";
	}
        outputFile.close();
        return true;
      }
      else if (!readFuncSymbol.empty()){
        std::ifstream inputFile;
        inputFile.open(readFuncSymbol);
        std::unordered_set<std::string> callerSymbols;
        std::string line;
        while (std::getline(inputFile, line)){
          callerSymbols.insert(line);
        }

        for (auto F = M.begin(); F!=M.end(); F++){
          if (callerSymbols.find(F->getName().str())!=callerSymbols.end()){
            if (F->empty()) continue;
            bool findDebugInfo = false;
            for (Function::iterator BBB = F->begin(), BBE = F->end(); BBB != BBE; ++BBB){
              for (BasicBlock::iterator IB = BBB->begin(), IE = BBB->end(); IB != IE; IB++){
                Instruction* I = dyn_cast<Instruction>(IB);
                if(DILocation *Loc = I->getDebugLoc()){
                  llvm::SmallString<128> relPath = Loc->getFilename();
		  llvm::SmallString<128> currentPath;
		  llvm::sys::fs::current_path(currentPath);
                  llvm::SmallString<128> AbsolutePath;
		  if ((!relPath.str().empty()) && (relPath.str()[0]!='/')){
                    std::string tmp;
 		    tmp = std::string(currentPath.str())+"/"+std::string(relPath.str());
                    llvm::sys::fs::real_path(tmp,AbsolutePath);
		  }

                  if((std::string(relPath)=="../ComposePostService/../tracing.h") ||
                     (std::string(relPath)=="../ComposePostService/../logger.h") ||
                     (std::string(relPath)=="../ComposePostService/../utils.h")) {
                    Function* func = dyn_cast<Function>(F);
		    errs()<<"@@@ current path: "<<currentPath.str()<<"\n";
                    errs()<<"@@@ "<<func->getName().str()<<", filename = "<<AbsolutePath.str()<<"\n";
                    std::string name = func->getName().str() + "_redundant";
           	    func->setName(name);
                  }
                  findDebugInfo = true;
                  break;
                }
	      }
              if (findDebugInfo) break;
            }
          }
        }

        for (auto F = M.begin();F!=M.end() ;F++){
	  if ((F->getName()=="faas_init") ||
	      (F->getName()=="faas_destroy_func_worker") || 
 	      (F->getName()=="faas_create_func_worker") ||
              (F->getName()=="faas_func_call")){
            Function* func = dyn_cast<Function>(F);
            std::string name = func->getName().str() + "_callee_";
	    const char* new_name = name.c_str();
      	    func->setName(new_name);
	  }
        }
      }
      else {
        errs()<<"need either write-func-symbol or read-func-symbol file\n";
      }
      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };
}

char ChangeFuncNames::ID = 0;
static RegisterPass<ChangeFuncNames>
Y("ChangeFuncNames", "Change the function name of faas_func_call in callee, otherwise the new function cannot be merged into the same address space due to duplicate of the function sympols");





namespace {
  struct RemoveRedundant : public ModulePass {
    static char ID; 
    RemoveRedundant() : ModulePass(ID) {}
    bool checkFuncSuffix(std::string);

    bool runOnModule(Module &M) override {
      std::unordered_set<Function*> redundantFuncs;
      for (auto F = M.begin(); F!=M.end(); F++){
	std::string FuncName(F->getName());
	if (checkFuncSuffix(FuncName)) redundantFuncs.insert(dyn_cast<Function>(F));
      }

      CallGraph cg = CallGraph(M);
      std::vector<std::pair<Function*, Function*> > callerCalleePair;
      for ( CallGraph::const_iterator itr = cg.begin(), ie = cg.end() ; itr != ie; itr++) {
        if (itr->second != nullptr){
	  CallGraphNode *callerNode = itr->second.get();
	  if (Function* callerFunc = callerNode->getFunction()){
            std::string callerName (callerFunc->getName());
      	    for (auto it = callerNode->begin(); it != callerNode->end(); it++) {
              if (CallGraphNode * calleeNode = it->second){
	        if (Function* fptr = calleeNode->getFunction()){
	          if (redundantFuncs.find(fptr) != redundantFuncs.end()){
		    callerCalleePair.push_back(std::make_pair(callerFunc, fptr));
		//    errs()<<"caller: "<<callerFunc->getName()<<", callee: "<<fptr->getName()<<"\n";
	          }
		}
	      }
	    }
	  }
	}
      }

      for (auto pair : callerCalleePair){
        Function* caller = pair.first;
        Function* oldCallee = pair.second;
	std::string oldFuncName(oldCallee->getName());
	std::string newFuncName = oldFuncName.substr(0, oldFuncName.size()-10);
        Function* newCallee = M.getFunction(newFuncName.c_str());

	errs()<<"###### "<<newCallee->getName()<<"\n";
        errs()<<"###### "<<oldCallee->getName()<<"\n";

	std::vector<CallInst*> callToBeChanged;
	std::vector<InvokeInst*> invokeToBeChanged;
 	for (Function::iterator BBB = caller->begin(), BBE = caller->end(); BBB != BBE; ++BBB){
	  for (BasicBlock::iterator IB = BBB->begin(), IE = BBB->end(); IB != IE; IB++){
            Instruction* I = dyn_cast<Instruction>(IB);
	    if (isa<CallInst>(I)){
              CallInst* callInst = dyn_cast<CallInst>(I);
	      Function* calleeFunc = callInst->getCalledFunction();
	      if ((calleeFunc) && (calleeFunc==pair.second)){
		callToBeChanged.push_back(callInst);
	      }
	    }
	    else if (isa<InvokeInst>(I)){
              InvokeInst* invokeInst = dyn_cast<InvokeInst>(I);
	      Function* invokedFunc = invokeInst->getCalledFunction();
              if ((invokedFunc) && (invokedFunc==pair.second)){
		invokeToBeChanged.push_back(invokeInst);
	      }
	    }
	  }
	}
	for (auto callInst: callToBeChanged){
	  std::vector<Value*> arguments;
          std::vector<Type*> argumentTypes;

	  for (unsigned i=0; i<callInst->getNumOperands()-1; i++){
            arguments.push_back(callInst->getOperand(i));
	    argumentTypes.push_back(callInst->getOperand(i)->getType());
          }

          ArrayRef<Type*> argTypes(argumentTypes);
          ArrayRef<Value*> args(arguments);
          CallInst* newCall = CallInst::Create(newCallee->getFunctionType(), newCallee, args ,"", callInst->getNextNode());

          Value* DestRPCInst = dyn_cast<Value>(callInst);
          for(auto U : DestRPCInst->users()){ 
            for (auto op = U->op_begin(); op != U->op_end(); op++){
              Value* op_value = dyn_cast<Value>(op);
              if (op_value == DestRPCInst){
                Value* DestNewCall = dyn_cast<Value>(newCall);
                *op = DestNewCall;
	      }
            }
          }
          callInst->eraseFromParent();
        }

	for (auto invokeInst: invokeToBeChanged){
	  BasicBlock* normalDest = invokeInst->getNormalDest();
          BasicBlock* unwindDest = invokeInst->getUnwindDest();

      	  std::vector<Value*> arguments;
          std::vector<Type*> argumentTypes;

	  for (unsigned i=0; i<invokeInst->getNumOperands()-3; i++){
            arguments.push_back(invokeInst->getOperand(i));
	    argumentTypes.push_back(invokeInst->getOperand(i)->getType());
          }

	  errs()<<"@@@@@ "<<*invokeInst<<"\n";

          ArrayRef<Type*> argTypes(argumentTypes);
          ArrayRef<Value*> args(arguments);
          InvokeInst* newInvoke = InvokeInst::Create(newCallee->getFunctionType(), newCallee, normalDest, unwindDest, args ,"", invokeInst);

          Value* DestRPCInst = dyn_cast<Value>(invokeInst);
          for(auto U : DestRPCInst->users()){ 
            for (auto op = U->op_begin(); op != U->op_end(); op++){
              Value* op_value = dyn_cast<Value>(op);
              if (op_value == DestRPCInst){
                Value* DestNewCall = dyn_cast<Value>(newInvoke);
                *op = DestNewCall;
	      }
            }
          }
          invokeInst->eraseFromParent();
        }
      }
      for (const auto& func: redundantFuncs) {
	func->eraseFromParent();
      }
      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };

  bool RemoveRedundant::checkFuncSuffix(std::string FuncName) {
    int len = FuncName.size();
    if (len<=10) return false;
    std::string suffix = FuncName.substr(len-10, 10);
    if (suffix == "_redundant") return true;
    else return false;
  } 
}

char RemoveRedundant::ID = 0;
static RegisterPass<RemoveRedundant>
Z("RemoveRedundant", "Change the function name of faas_func_call in callee, otherwise the new function cannot be merged into the same address space due to duplicate of the function sympols");

