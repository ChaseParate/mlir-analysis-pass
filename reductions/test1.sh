#!/usr/bin/env bash

PLUGIN="../build/FPANAnalysis.so"
PIPELINE="builtin.module(fpan-analysis)"

mlir-translate --import-llvm "$1" \
    | mlir-opt --load-pass-plugin="$PLUGIN" --pass-pipeline="$PIPELINE" -o /dev/null 2>&1 \
    | grep -qE 'llvm\.extractvalue .+? // .+? is {\+0}'
