
include_directories(include)
link_directories(lib)

add_llvm_library( LLVMMetaTrans MODULE
  Filter.cpp
  MetaFilter.cpp
  MetaPass.cpp
  MetaTrans.cpp
  MetaUtils.cpp
  MetaMatcher.cpp

  PLUGIN_TOOL
  opt
)
add_dependencies(LLVMMetaTrans intrinsics_gen)
target_link_libraries(LLVMMetaTrans libyaml-cpp.a)