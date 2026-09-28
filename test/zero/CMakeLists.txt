add_test(NAME zero-analysis
  COMMAND ${CMAKE_COMMAND}
    -DMLIR_OPT=${MLIR_OPT_EXECUTABLE}
    -DPLUGIN=$<TARGET_FILE:ZeroAnalysis>
    -DPIPELINE=builtin.module\(zero-analysis\)
    -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/zero.mlir
    -DCHECKS=${CMAKE_CURRENT_SOURCE_DIR}/zero.expected
    -P ${PROJECT_SOURCE_DIR}/cmake/RunTest.cmake)
