#!/bin/bash

NIGHTCORE_PATH=/proj/zyuxuanssf-PG0/nightcore
LLVM_PATH=/proj/zyuxuanssf-PG0/llvm-project
NIGHTCORE_TEST_PATH=/proj/zyuxuanssf-PG0/nightcore-test/socialnetwork_singlenode/DeathStarBench

rm *.ll

# compile the code into LLVM IR
#clang++ -I$NIGHTCORE_TEST_PATH/socialNetwork/src -fPIC -emit-llvm -gmodules -S $NIGHTCORE_TEST_PATH/socialNetwork/src/UniqueIdService/UniqueIdService.cpp -c -o UniqueId.ll
clang++ -I$NIGHTCORE_TEST_PATH/socialNetwork/src -fPIC -emit-llvm -g -S $NIGHTCORE_TEST_PATH/socialNetwork/src/UniqueIdService/UniqueIdService.cpp -c -o UniqueId.ll

clang++ -I$NIGHTCORE_TEST_PATH/socialNetwork/src -fPIC -emit-llvm -gmodules -S $NIGHTCORE_TEST_PATH/socialNetwork/gen-cpp/UniqueIdService.cpp -c -o gen-cpp-UniqueId.ll

clang++ -I$NIGHTCORE_TEST_PATH/socialNetwork/src -fPIC -emit-llvm -g -S $NIGHTCORE_TEST_PATH/socialNetwork/src/ComposePostService/ComposePostService.cpp -c -o ComposePost.ll

# change function name "faas_func_call" in bar to be "faas_func_callee"
# also delete "faas_init", "faas_create_func_worker" and "faas_destroy_func_worker"
opt -load $LLVM_PATH/build/lib/LLVMMergeFuncReal.so -enable-new-pm=0 -ChangeFuncNames -write-func-symbol=func_sym.txt UniqueId.ll -S -o test_virt_call.ll

opt -load $LLVM_PATH/build/lib/LLVMMergeFuncReal.so -enable-new-pm=0 -ChangeFuncNames -read-func-symbol=func_sym.txt ComposePost.ll -S -o ComposePost_new.ll

# merge "faas_func_callee" into foo's address space
llvm-link UniqueId.ll ComposePost_new.ll -o merged.ll -S
opt merged.ll -strip-debug -o merged_no_debuginfo.ll -S
#opt -load $LLVM_PATH/build/lib/LLVMMergeFuncReal.so -enable-new-pm=0 -MergeFuncReal -S -o merged_update.ll < merged_no_debuginfo.ll
opt -load $LLVM_PATH/build/lib/LLVMMergeFuncReal.so -enable-new-pm=0 -RemoveRedundant -S -o merged_update.ll < merged_no_debuginfo.ll

# change the "faas_func_callee" to be "Bar" (normal function call)
#opt -load $LLVM_PATH/build/lib/LLVMMergeFunc.so -S -enable-new-pm=0 -o foo_bar.ll -MergeFunc < foo_rpc_bar.ll

# finally generate obj of libfoo and link the .obj file
#llc -filetype=obj -relocation-model=pic foo_bar.ll -o libfoo.o
#clang -shared -fPIC -O2 -I../../include libfoo.o -o libfoo.so 
