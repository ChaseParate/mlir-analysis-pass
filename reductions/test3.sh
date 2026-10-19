#!/usr/bin/env bash

PLUGIN="../build/FPANAnalysis.so"
PIPELINE="builtin.module(fpan-analysis)"

annotated=$(mlir-translate --import-llvm "$1" \
    | mlir-opt --load-pass-plugin="$PLUGIN" --pass-pipeline="$PIPELINE" -o /dev/null 2>&1)

# Four calls, each with an immediate sum and error extraction.  SE-S1 retains
# the exponent -24 identity possibility, and Z2 propagates that abstract error
# state through the final call.
[ "$(grep -cE 'llvm\.extractvalue .*// .* is ' <<< "$annotated")" -eq 8 ] &&
    grep -qE 'llvm\.extractvalue .*// .* is .*e = -24' <<< "$annotated"
