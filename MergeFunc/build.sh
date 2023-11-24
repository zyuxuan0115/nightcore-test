#!/bin/bash

NIGHTCORE_PATH=/proj/zyuxuanssf-PG0/nightcore
LLVM_PATH=/proj/zyuxuanssf-PG0/llvm-project

rm *.so *.o *.ll
clang -I$NIGHTCORE_PATH/include -fPIC -emit-llvm -S foo.c -c -o foo.ll
clang -I$NIGHTCORE_PATH/include -fPIC -emit-llvm -S bar.c -c -o bar.ll
opt -load $LLVM_PATH/build/lib/LLVMMergeFunc.so -enable-new-pm=0 -ChangeFuncName bar.ll -S -o bar_func_only.ll
llvm-link foo.ll bar_func_only.ll -o foo_bar.ll -S
opt -load $LLVM_PATH/build/lib/LLVMMergeFunc.so -S -enable-new-pm=0 -o new_foo.ll -ConvertRPC2NormalCall < foo_bar.ll
llc -filetype=obj -relocation-model=pic new_foo.ll -o libfoo.o
clang -shared -fPIC -O2 -I../../include libfoo.o -o libfoo.so 
