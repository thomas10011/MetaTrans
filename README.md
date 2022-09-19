# MetaTrans

由于使用了yaml-cpp，所以需要在LLVM的编译选项中去掉`-fno-exceptions`和`-fno-rtti`。从AddLLVM.cmake文件中可知，加上`LLVM_REQUIRES_EH`编译选项即可。

当前版本基于哈希为`fac0fb4d966efe8c70d3e566cca6a5d0bd049302`的commit可以正常运行。