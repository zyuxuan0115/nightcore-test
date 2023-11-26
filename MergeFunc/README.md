## LLVM pass to convert RPCs into normal calls
Only works for [nightcore](https://github.com/ut-osa/nightcore)'s C/C++ serverless functions

#### Build MergeFunc pass

- setup LLVM ([llvm-13.0.1 source code (tar.gz)](https://github.com/llvm/llvm-project/releases/tag/llvmorg-13.0.1))
  + `echo 'export PATH=/proj/zyuxuanssf-PG0/llvm-project/build/bin:$PATH' >> ~/.bashrc` in the following script needs to be changed.

```bash
> wget https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-13.0.1.tar.gz
> tar -vxf llvmorg-13.0.1.tar.gz
> mv llvm-project-llvmorg-13.0.1 llvm-project && cd llvm-project
> mkdir build && cd build
> cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Release" -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lldb;lld" DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" ../llvm
> make -j
> echo 'export PATH=/proj/zyuxuanssf-PG0/llvm-project/build/bin:$PATH' >> ~/.bashrc
> source ~/.bashrc
```

- build the `MergeFunc.so` pass
  + `NIGHTCORE_TEST_PATH=/proj/zyuxuanssf-PG0` & `LLVM_PATH=/proj/zyuxuanssf-PG0` in the following script need to be changed. 

```bash
> git clone https://github.com/zyuxuan0115/nightcore-test.git
> NIGHTCORE_TEST_PATH=/proj/zyuxuanssf-PG0
> LLVM_PATH=/proj/zyuxuanssf-PG0
> cp -r $NIGHTCORE_TEST_PATH/nightcore-test/MergeFunc $LLVM_PATH/llvm-project/llvm/lib/Transforms/
> echo 'add_subdirectory(MergeFunc)' >> $LLVM_PATH/llvm-project/llvm/lib/Transforms/CMakeList.txt
> cd $LLVM_PATH/llvm-project/build && make -j
```

- run the pass to merge function `foo` and `bar`
  + build nightcore by following the instructions in this [webpage](https://github.com/ut-osa/nightcore)
  + the path of `LLVM_PATH` and `NIGHTCORE_PATH` in [build.sh](https://github.com/zyuxuan0115/nightcore-test/blob/main/MergeFunc/build.sh) need to be changed

```bash
> git clone --recurse-submodules https://github.com/zyuxuan0115/nightcore.git
> NIGHTCORE_PATH=/proj/zyuxuanssf-PG0 
> cd $NIGHTCORE_PATH/nightcore/examples/c_merge/
> cp $NIGHTCORE_TEST_PATH/nightcore-test/MergeFunc/build.sh .
> sudo chmod +x build.sh
> ./build.sh
```

- test it with nightcore
  + after running the following commands, you should see `From function Bar: Hello, World` in terminal
```bash
> ./run_stack.sh &
> curl -X POST -d "Hello" http://127.0.0.1:8080/function/Foo
```

#### Some notes
- how to build LLVM passes
  + [write an LLVM pass](https://llvm.org/docs/WritingAnLLVMPass.html) from LLVM's website
  + a simpler tutorial from UTAustin [here](https://www.cs.utexas.edu/~pingali/CS380C/2020/assignments/llvm-guide.html)

- the meaning of some of the flags in `build.sh`
  + `-g` adds general debug info to IR. (might be useful for identifying the virtual function call)
  + `opt` needs to run with the `-enable-new-pm=0` flag since I still use the legacy llvm pass
  + `llvm-link` needs to run with the `--relocation-model=pic` flag, otherwise clang will report `relocation R_X86_64_32S against '.data' can not be used when making a shared object; recompile with -fPIC` 

- to test whether there is a function "Bar" in the address space of libfoo.so
  + first run `objdump -d libfoo.so | less`
  + then search for `Bar`




