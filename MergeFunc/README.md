### LLVM pass to convert PRC into normal calls

- setup LLVM ([llvm-10.0 source code (tar.gz)](https://github.com/llvm/llvm-project/releases/tag/llvmorg-10.0.0))

```bash
> git clone https://github.com/llvm/llvm-project.git
> cd llvm-project
> mkdir build && cd build
>cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Release" -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lldb;lld" DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" ../llvm
> make -j
```

- build passes
link from UTAustin [here](https://www.cs.utexas.edu/~pingali/CS380C/2020/assignments/llvm-guide.html)

```bash
> cp -r /proj/zyuxuanssf-PG0/nightcore-test/MergeFunc /proj/zyuxuanssf-PG0/llvm-project/llvm/lib/Transforms/
> echo 'add_subdirectory(MergeFunc)' >> /proj/zyuxuanssf-PG0/llvm-project/llvm/lib/CMakeList.txt
> cd llvm-project/build && make -j
```

- in `nightcore/examples/c`
  + `-g` adds general debug info to IR. might be useful for identify the virtual function call
  
```bash
> clang -I/proj/zyuxuanssf-PG0/nightcore/include -g -emit-llvm foo.c -c -o foo.bc
> clang -I/proj/zyuxuanssf-PG0/nightcore/include -g -emit-llvm bar.c -c -o bar.bc
> opt -load /proj/zyuxuanssf-PG0/llvm-project/build/lib/LLVMMergeFunc.so -ChangeFuncName bar.bc -o bar_func_only.bc
> llvm-link foo.bc bar_func_only.bc -o foo_bar.ll -S
> opt -load /proj/zyuxuanssf-PG0/llvm-project/build/lib/LLVMMergeFunc.so -S -o new_foo.ll -MergeFunc < foo_bar.ll 
```


