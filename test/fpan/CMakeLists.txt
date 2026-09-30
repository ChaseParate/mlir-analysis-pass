function(add_fpan_analysis_test name input checks)
  add_test(
    NAME fpan-analysis::${name}
    COMMAND ${CMAKE_COMMAND}
      -DMLIR_OPT=${MLIR_OPT_EXECUTABLE}
      -DPLUGIN=$<TARGET_FILE:FPANAnalysis>
      -DPIPELINE=builtin.module\(fpan-analysis\)
      -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/${input}
      -DCHECKS=${CMAKE_CURRENT_SOURCE_DIR}/${checks}
      -P ${PROJECT_SOURCE_DIR}/cmake/RunTest.cmake
  )
endfunction()

add_fpan_analysis_test(
  constants
  constants.mlir
  constants.expected
)

add_fpan_analysis_test(
  lemma_family_z
  lemma_family_z.mlir
  lemma_family_z.expected
)
