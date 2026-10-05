#!/usr/bin/env bash

PLUGIN="../build/FPANAnalysis.so"
PIPELINE="builtin.module(fpan-analysis)"

annotated=$(mlir-translate --import-llvm "$1" \
    | mlir-opt --load-pass-plugin="$PLUGIN" --pass-pipeline="$PIPELINE" -o /dev/null 2>&1)

# Three calls, each with an immediate sum and error extraction.
[ "$(grep -cE 'llvm\.extractvalue .*// .* is \{\+0\}' <<< "$annotated")" -eq 6 ]
