
# MetaTrans

> [!NOTE]
> Since most of the codes in this repository has been integrated into the main repository, it's currently in archive status.

## How to BUILD && RUN
1. Clone llvm project.
2. Clone code into `llvm-project/llvm/lib/Transforms/`, so the full relative path should be `llvm-project/llvm/lib/Transforms/MetaTrans`.
3. (Not Necessary)Set your compiler path in `llvm-project/llvm/lib/Transforms/MetaTrans/CMakeLists.txt`
4. Append `add_subdirectory(MetaTrans)` to `llvm-project/llvm/lib/Transforms/CMakeLists.txt`
5. Build llvm.
6. Run opt with the generated dynamic library (LLVMMetaTrans.dylib in macos or LLVMMetaTrans.so in Linux). 

**Example command**

Using following command to compile the LLVM project into debug mode.

```
cmake -S llvm -B build -G "Ninja" -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_TARGETS_TO_BUILD="X86;RISCV;ARM" -DLLVM_ENABLE_ASSERTIONS=true -DCMAKE_BUILD_TYPE=Debug -DLLVM_REQUIRES_EH=true
```

And then run the pass.

`~/Projects/llvm-project/build/bin/opt -enable-new-pm=0 -load ~/Projects/llvm-project/build/lib/LLVMMetaTrans.dylib --meta-trans test.ll`

`-enable-new-pm=0` means do not use new pass manager, `--meta-trans` invoke the meta translation pass.





## How to DEBUG

If you are using lldb, you can follow this (substitute the binary path to yours):

```
lldb -- ~/projects/llvm-project-dev/build/bin/opt -debug -enable-new-pm=0 -load ~/projects/llvm-project-dev/build/lib/LLVMMetaTrans.so --meta-trans example.ll 
```

this should run lldb with opt, and load the pass as a shared object. The output will be like:

```
(lldb) target create "/work/stu/hrtan/projects/llvm-project-dev/build/bin/opt"
Current executable set to '/work/stu/hrtan/projects/llvm-project-dev/build/bin/opt' (x86_64).
(lldb) settings set -- target.run-args  "-debug" "-enable-new-pm=0" "-load" "/work/stu/hrtan/projects/llvm-project-dev/build/lib/LLVMMetaTrans.so" "--meta-trans" "wshift.ll"
```

Everything is now set up! Now you can run and set your breakpoint like:

```
(lldb) breakpoint set --file MetaPass.cpp --line 472
(lldb) run
```

lldb's execution flow should stop at the breakpoint.


## Notice

We used yaml-cpp, so you need to kick off `-fno-exceptions` and `-fno-rtti` from your compilation options, and for the polymorphic feature required by yaml-cpp, you need to add `LLVM_REQUIRES_EH` option.


All tests is under LLVM 13.0.1.
