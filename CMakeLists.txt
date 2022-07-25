add_llvm_library( LLVMExample MODULE
  CountOpcode.cpp
  MetaInst.cpp

  PLUGIN_TOOL
  opt
)