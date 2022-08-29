SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")
add_llvm_library( LLVMMetaTrans MODULE
  CountOpcode.cpp
  MetaTrans.cpp

  PLUGIN_TOOL
  opt
)