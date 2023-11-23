NIGHTCORE_PATH=/proj/zyuxuanssf-PG0/nightcore
LLVM_PATH=/proj/zyuxuanssf-PG0/llvm-project

rm *.so *.o *.bc *.ll *.s
clang -I$NIGHTCORE_PATH/include -fPIC -emit-llvm foo.c -c -o foo.bc
clang -I$NIGHTCORE_PATH/include -fPIC -emit-llvm bar.c -c -o bar.bc
opt -load $LLVM_PATH/build/lib/LLVMMergeFunc.so -enable-new-pm=0 -ChangeFuncName bar.bc -o bar_func_only.bc
llvm-link foo.bc bar_func_only.bc -o foo_bar.ll -S
opt -load $LLVM_PATH/build/lib/LLVMMergeFunc.so -S -enable-new-pm=0 -o new_foo.ll -MergeFunc < foo_bar.ll
llc -filetype=obj -relocation-model=pic new_foo.ll -o libfoo.o
clang -shared -fPIC -O2 -I../../include libfoo.o -o libfoo.so 
