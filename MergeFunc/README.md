## LLVM pass to convert PRCs into normal calls
Only works for [nightcore](https://github.com/ut-osa/nightcore)'s C/C++ serverless functions

#### build MergeFunc pass

- setup LLVM ([llvm-13.0.1 source code (tar.gz)](https://github.com/llvm/llvm-project/releases/tag/llvmorg-13.0.1))
  + `LLVM_PATH=/proj/zyuxuanssf-PG0` in the following script needs to be changed.

```bash
> wget https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-13.0.1.tar.gz
> tar -vxf llvmorg-13.0.1.tar.gz
> mv llvm-project-llvmorg-13.0.1 llvm-project && cd llvm-project
> mkdir build && cd build
> cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Release" -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lldb;lld" DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" ../llvm
> make -j
> LLVM_PATH=/proj/zyuxuanssf-PG0 
> echo "export PATH=$LLVM_PATH/llvm-project/build/bin:$PATH" >> ~/.bashrc
> source ~/.bashrc
```

- build the `MergeFunc.so` pass
  + `NIGHTCORE_TEST_PATH=/proj/zyuxuanssf-PG0` in the following script needs to be changed. 

```bash
> git clone https://github.com/zyuxuan0115/nightcore-test.git
> NIGHTCORE_TEST_PATH=/proj/zyuxuanssf-PG0 
> cp -r $NIGHTCORE_TEST_PATH/nightcore-test/MergeFunc $LLVM_PATH/llvm-project/llvm/lib/Transforms/
> echo 'add_subdirectory(MergeFunc)' >> $LLVM_PATH/llvm-project/llvm/lib/Transforms/CMakeList.txt
> cd $LLVM_PATH/llvm-project/build && make -j
```

- run the pass to merge function `foo` and `bar`
  + the path of `LLVM_PATH` and `NIGHTCORE_PATH` in [build.sh](https://github.com/zyuxuan0115/nightcore-test/blob/main/MergeFunc/build.sh) need to be changed

```bash
> git clone --recurse-submodules git@github.com:ut-osa/nightcore.git
> cd $NIGHTCORE_TEST_PATH/nightcore-test/MergeFunc
> sudo chmod +x build.sh
> ./build.sh
```

#### Some notes
- how to build LLVM passes
  + [write an LLVM pass](https://llvm.org/docs/WritingAnLLVMPass.html) from LLVM's website
  + a simpler tutorial from UTAustin [here](https://www.cs.utexas.edu/~pingali/CS380C/2020/assignments/llvm-guide.html)

- the meaning of some of the flags in `build.sh`
  + `-g` adds general debug info to IR. (might be useful for identify the virtual function call)
  + `opt` needs to run with the `-enable-new-pm=0` flag since I still use the legacy llvm pass
  + `llvm-link` needs to run with the `--relocation-model=pic` flag, otherwise clang will report `relocation R_X86_64_32S against '.data' can not be used when making a shared object; recompile with -fPIC` 


