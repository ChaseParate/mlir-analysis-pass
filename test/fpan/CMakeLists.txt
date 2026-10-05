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

function(add_fpan_llvm_import_analysis_test name input checks)
  add_test(
    NAME fpan-analysis::${name}
    COMMAND ${CMAKE_COMMAND}
      -DMLIR_OPT=${MLIR_OPT_EXECUTABLE}
      -DMLIR_TRANSLATE=${MLIR_TRANSLATE_EXECUTABLE}
      -DPLUGIN=$<TARGET_FILE:FPANAnalysis>
      -DPIPELINE=builtin.module\(fpan-analysis\)
      -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/${input}
      -DGENERATED_INPUT=${CMAKE_CURRENT_BINARY_DIR}/${name}.mlir
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

add_fpan_analysis_test(
  lemma_family_se_i
  lemma_family_se_i.mlir
  lemma_family_se_i.expected
)

add_fpan_analysis_test(
  cfg_join
  cfg_join.mlir
  cfg_join.expected
)

if(ENABLE_LLVM_IMPORT_INTEGRATION_TESTS)
  add_fpan_llvm_import_analysis_test(
    llvm_aggregate_call
    llvm_aggregate_call.ll
    llvm_aggregate_call.expected
  )
endif()
