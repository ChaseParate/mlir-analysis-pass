add_test(NAME fpan-analysis
  COMMAND ${CMAKE_COMMAND}
    -DMLIR_OPT=${MLIR_OPT_EXECUTABLE}
    -DPLUGIN=$<TARGET_FILE:FPANAnalysis>
    -DPIPELINE=builtin.module\(fpan-analysis\)
    -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/basic.mlir
    -DCHECKS=${CMAKE_CURRENT_SOURCE_DIR}/basic.expected
    -P ${PROJECT_SOURCE_DIR}/cmake/RunTest.cmake)
