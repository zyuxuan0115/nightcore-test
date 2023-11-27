#!/bin/bash

NIGHTCORE_PATH=/proj/zyuxuanssf-PG0/nightcore
LLVM_PATH=/proj/zyuxuanssf-PG0/llvm-project

rm *.so *.o *.ll

# compile the code into LLVM IR
clang -I$NIGHTCORE_PATH/include -fPIC -emit-llvm -S $NIGHTCORE_PATH/examples/c_merge/foo.c -c -o foo.ll
clang -I$NIGHTCORE_PATH/include -fPIC -emit-llvm -S $NIGHTCORE_PATH/examples/c_merge/bar.c -c -o bar.ll

# change function name "faas_func_call" in bar to be "faas_func_callee"
# also delete "faas_init", "faas_create_func_worker" and "faas_destroy_func_worker"
opt -load $LLVM_PATH/build/lib/LLVMMergeFunc.so -enable-new-pm=0 -ChangeFuncName bar.ll -S -o bar_only.ll

# merge "faas_func_callee" into foo's address space
llvm-link foo.ll bar_only.ll -o foo_rpc_bar.ll -S

# change the "faas_func_callee" to be "Bar" (normal function call)
opt -load $LLVM_PATH/build/lib/LLVMMergeFunc.so -S -enable-new-pm=0 -o foo_bar.ll -MergeFunc < foo_rpc_bar.ll

# finally generate obj of libfoo and link the .obj file
llc -filetype=obj -relocation-model=pic foo_bar.ll -o libfoo.o
clang -shared -fPIC -O2 -I../../include libfoo.o -o libfoo.so 
