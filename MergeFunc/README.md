### LLVM pass to change PRC into normal calls

- setup LLVM
  + get [llvm-10.0 source code (tar.gz)](https://github.com/llvm/llvm-project/releases/tag/llvmorg-10.0.0)

```bash
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
mkdir build && cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Release" -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lldb;lld" DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" ../llvm
make -j 4
```

- build passes
link from UTAustin [here](https://www.cs.utexas.edu/~pingali/CS380C/2020/assignments/llvm-guide.html)

- in `nightcore/examples/c`
  ```bash
  > clang -I/proj/zyuxuanssf-PG0/nightcore/include -emit-llvm -S foo.c
  > opt -load /proj/zyuxuanssf-PG0/llvm-project/build/lib/LLVMMergeFunc.so -MergeFunc < foo.ll > /dev/null
  ```
