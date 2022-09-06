SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")

include_directories(include)
link_directories(lib)

add_llvm_library( LLVMMetaTrans MODULE
  MetaPass.cpp
  MetaTrans.cpp
  utils.cpp

  PLUGIN_TOOL
  opt
)
target_link_libraries(LLVMMetaTrans yaml-cpp)