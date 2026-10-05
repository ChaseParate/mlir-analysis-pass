#!/usr/bin/env bash

PLUGIN="../build/FPANAnalysis.so"
PIPELINE="builtin.module(fpan-analysis)"

annotated=$(mlir-translate --import-llvm "$1" \
    | mlir-opt --load-pass-plugin="$PLUGIN" --pass-pipeline="$PIPELINE" -o /dev/null 2>&1)

# Four calls, each with an immediate sum and error extraction; SE-I preserves
# the 2^-24 operand and Z2 propagates it through the final call.
[ "$(grep -cE 'llvm\.extractvalue .*// .* is ' <<< "$annotated")" -eq 8 ] &&
  grep -qE 'llvm\.extractvalue .*// .* is \{\(s = \+, e = -24\)\}' <<< "$annotated"
