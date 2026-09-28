#!/bin/sh
# Run an analysis over an MLIR file.
#
#   ./run.sh [--analysis fpan|zero] input.mlir
#
# FPAN is the default. Set ANALYSIS, PLUGIN, or BUILD_DIR to override.
set -eu

BUILD_DIR="${BUILD_DIR:-build}"
ANALYSIS="${ANALYSIS:-fpan}"

if [ "${1:-}" = "--analysis" ]; then
  if [ "$#" -lt 2 ]; then
    echo "usage: $0 [--analysis fpan|zero] input.mlir [mlir-opt options...]" >&2
    exit 2
  fi
  ANALYSIS="$2"
  shift 2
fi

case "$ANALYSIS" in
  fpan)
    plugin_name=FPANAnalysis
    pipeline='builtin.module(fpan-analysis)'
    ;;
  zero)
    plugin_name=ZeroAnalysis
    pipeline='builtin.module(zero-analysis)'
    ;;
  *)
    echo "Unknown analysis '$ANALYSIS'; expected fpan or zero." >&2
    exit 2
    ;;
esac

if [ -z "${PLUGIN:-}" ]; then
  for candidate in "$BUILD_DIR"/"$plugin_name".so "$BUILD_DIR"/"$plugin_name".dylib; do
    if [ -f "$candidate" ]; then
      PLUGIN="$candidate"
      break
    fi
  done
fi

if [ -z "${PLUGIN:-}" ]; then
  echo "No $plugin_name plugin found in $BUILD_DIR; build it first (see README.md)." >&2
  exit 1
fi

if [ "$#" -lt 1 ]; then
  echo "usage: $0 [--analysis fpan|zero] input.mlir [mlir-opt options...]" >&2
  exit 2
fi

# stdout is the unchanged IR and stderr is the annotated listing; send the
# listing to this script's stdout so it can be piped or paged.
mlir-opt --load-pass-plugin="$PLUGIN" \
         --pass-pipeline="$pipeline" \
         "$@" 2>&1 1>/dev/null
