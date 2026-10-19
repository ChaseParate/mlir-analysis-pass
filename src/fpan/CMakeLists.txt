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

# A small end-to-end harness for the assertion instrumentation.  It converts
# the func-dialect S1 fixture to LLVM before FPAN runs, because the current
# instrumentation emits llvm.call operations.  The resulting executable links
# both the concrete TwoSum implementation and the assertion runtime.
find_program(FPAN_MLIR_OPT_EXECUTABLE mlir-opt
  HINTS ${MLIR_TOOLS_BINARY_DIR} ${LLVM_TOOLS_BINARY_DIR})
find_program(FPAN_MLIR_TRANSLATE_EXECUTABLE mlir-translate
  HINTS ${MLIR_TOOLS_BINARY_DIR} ${LLVM_TOOLS_BINARY_DIR})
find_program(FPAN_CLANGXX_EXECUTABLE clang++
  HINTS ${LLVM_TOOLS_BINARY_DIR})

if(FPAN_MLIR_OPT_EXECUTABLE AND FPAN_MLIR_TRANSLATE_EXECUTABLE AND FPAN_CLANGXX_EXECUTABLE)
  set(FPAN_DEMO_MLIR "${CMAKE_CURRENT_BINARY_DIR}/fpan-instrumentation-demo.mlir")
  set(FPAN_DEMO_LL "${CMAKE_CURRENT_BINARY_DIR}/fpan-instrumentation-demo.ll")
  set(FPAN_DEMO_EXE "${CMAKE_CURRENT_BINARY_DIR}/fpan-instrumentation-demo")

  add_custom_command(
    OUTPUT "${FPAN_DEMO_EXE}"
    COMMAND "${FPAN_MLIR_OPT_EXECUTABLE}"
      "--load-pass-plugin=$<TARGET_FILE:FPANAnalysis>"
      "--pass-pipeline=builtin.module(convert-func-to-llvm,fpan-analysis{instrument-asserts=true})"
      "${PROJECT_SOURCE_DIR}/test/fpan/instrumentation_demo.mlir"
      -o "${FPAN_DEMO_MLIR}"
    COMMAND "${FPAN_MLIR_TRANSLATE_EXECUTABLE}"
      --mlir-to-llvmir "${FPAN_DEMO_MLIR}" -o "${FPAN_DEMO_LL}"
    # mlir-translate produces textual LLVM IR; use clang++ rather than the
    # project compiler because a GCC-configured tree cannot consume .ll files.
    COMMAND "${FPAN_CLANGXX_EXECUTABLE}"
      -O0 -fno-fast-math -ffp-contract=off
      "${FPAN_DEMO_LL}"
      "${CMAKE_CURRENT_SOURCE_DIR}/instrumentation/FPANAssert.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/instrumentation/FPANAssertDemo.cpp"
      -o "${FPAN_DEMO_EXE}"
    DEPENDS
      FPANAnalysis
      "${PROJECT_SOURCE_DIR}/test/fpan/instrumentation_demo.mlir"
      "${CMAKE_CURRENT_SOURCE_DIR}/instrumentation/FPANAssert.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/instrumentation/FPANAssertDemo.cpp"
    VERBATIM)

  add_custom_target(fpan-instrumentation-demo DEPENDS "${FPAN_DEMO_EXE}")
  add_custom_target(run-fpan-instrumentation-demo
    COMMAND "${FPAN_DEMO_EXE}"
    DEPENDS fpan-instrumentation-demo
    USES_TERMINAL)
else()
  message(STATUS "mlir-opt, mlir-translate, or clang++ not found; skipping fpan-instrumentation-demo")
endif()
