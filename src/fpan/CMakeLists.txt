# MODULE + PLUGIN_TOOL means: build a loadable module that deliberately leaves
# its LLVM/MLIR symbols undefined, to be satisfied by the mlir-opt process
# that dlopens it.  Linking MLIR's static libraries in here instead would
# duplicate every cl::opt and dialect registry, which fails at load time.
add_llvm_library(FPANAnalysis MODULE PARTIAL_SOURCES_INTENDED
  FPANAnalysis.cpp
  FPANPlugin.cpp
  ../Annotate.cpp
  PLUGIN_TOOL mlir-opt
)

# FPANPlugin.cpp uses the shared annotation interface in src/.
target_include_directories(FPANAnalysis PRIVATE
  "${PROJECT_SOURCE_DIR}/src")

# Keep run.sh and the test's generated plugin path independent of the
# analysis subdirectory and of the platform's shared-library suffix.
set_target_properties(FPANAnalysis PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
