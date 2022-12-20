
# MetaTrans

## How to build && run
1. Clone llvm project.
2. Clone code into `llvm-project/llvm/lib/Transforms/`, so the full relative path should be `llvm-project/llvm/lib/Transforms/MetaTrans`.
3. (Not Necessary)Set your compiler path in `llvm-project/llvm/lib/Transforms/MetaTrans/CMakeLists.txt`
4. Append `add_subdirectory(MetaTrans)` to `llvm-project/llvm/lib/Transforms/CMakeLists.txt`
5. Build llvm.
6. Run opt with the generated dynamic library (LLVMMetaTrans.dylib in macos or LLVMMetaTrans.so in Linux). 

Example command
`~/Projects/llvm-project/build/bin/opt -enable-new-pm=0 -load ~/Projects/llvm-project/build/lib/LLVMMetaTrans.dylib --meta-trans test.ll`
-enable-new-pm=0 means do not use new pass manager, --meta-trans invoke the meta translation pass.


## Notice
由于使用了yaml-cpp，所以需要在LLVM的编译选项中去掉`-fno-exceptions`和`-fno-rtti`。从AddLLVM.cmake文件中可知，加上`LLVM_REQUIRES_EH`编译选项即可。

当前版本基于哈希为`fac0fb4d966efe8c70d3e566cca6a5d0bd049302`的commit可以正常运行。理论上来说，所有LLVM12的提交版本都可以正常运行。
