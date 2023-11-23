### LLVM pass to convert PRCs into normal calls
- only works for [nightcore](https://github.com/ut-osa/nightcore)'s C/C++ serverless functions
- setup LLVM ([llvm-13.0.1 source code (tar.gz)](https://github.com/llvm/llvm-project/releases/tag/llvmorg-13.0.1))

```bash
> wget https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-13.0.1.tar.gz
> tar -vxf llvmorg-13.0.1.tar.gz
> mv llvm-project-llvmorg-13.0.1 llvm-project && cd llvm-project
> mkdir build && cd build
> cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Release" -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lldb;lld" DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" ../llvm
> make -j
```

- build passes
  + link from UTAustin [here](https://www.cs.utexas.edu/~pingali/CS380C/2020/assignments/llvm-guide.html)

```bash
> cp -r {path to nightcore-test}/nightcore-test/MergeFunc {path to llvm-project}/llvm-project/llvm/lib/Transforms/
> echo 'add_subdirectory(MergeFunc)' >> {path to llvm-project}/llvm-project/llvm/lib/CMakeList.txt
> cd {path to llvm-project}/llvm-project/build && make -j
```

- in `nightcore/examples/c`
  + `-g` adds general debug info to IR. might be useful for identify the virtual function call
  + `-enable-new-pm=0` is added when compile the code since I still use the legacy llvm pass 
  
```bash
> cd nightcore/examples/c
> clang -I/proj/zyuxuanssf-PG0/nightcore/include -fPIC -emit-llvm foo.c -c -o foo.bc
> clang -I/proj/zyuxuanssf-PG0/nightcore/include -fPIC -emit-llvm bar.c -c -o bar.bc
> opt -load /proj/zyuxuanssf-PG0/llvm-project/build/lib/LLVMMergeFunc.so -enable-new-pm=0 -ChangeFuncName bar.bc -o bar_func_only.bc
> llvm-link foo.bc bar_func_only.bc -o foo_bar.ll -S
> opt -load /proj/zyuxuanssf-PG0/llvm-project/build/lib/LLVMMergeFunc.so -S -enable-new-pm=0 -o new_foo.ll -MergeFunc < foo_bar.ll
> llc -filetype=obj new_foo.ll -o libfoo.o
> clang -shared -fPIC -O2 -I../../include libfoo.o -o libfoo.so 
```


