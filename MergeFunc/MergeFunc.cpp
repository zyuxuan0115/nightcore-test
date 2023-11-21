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
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "MergeFunc"

STATISTIC(MergeFuncCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct MergeFunc : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    MergeFunc() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      ++MergeFuncCounter;
      errs() << "MergeFunc: ";
      errs().write_escaped(F.getName()) << '\n';
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
