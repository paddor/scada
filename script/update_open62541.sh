#!/usr/bin/env bash
set -euo pipefail

TAG="${1:-v1.4.6}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
DEPS_DIR="$ROOT_DIR/ext/scada/deps"
WORK_DIR="$(mktemp -d)"

cleanup() { rm -rf "$WORK_DIR"; }
trap cleanup EXIT

echo "==> Cloning open62541 @ $TAG"
git clone --depth 1 --branch "$TAG" https://github.com/open62541/open62541.git "$WORK_DIR/open62541"

echo "==> Building amalgamation"
cd "$WORK_DIR/open62541"
mkdir build && cd build
cmake .. \
  -DUA_ENABLE_AMALGAMATION=ON \
  -DUA_NAMESPACE_ZERO=FULL \
  -DUA_ENABLE_ENCRYPTION=ON \
  -DUA_ENABLE_ENCRYPTION_MBEDTLS=ON \
  -DCMAKE_BUILD_TYPE=Release
make open62541-amalgamation-source open62541-amalgamation-header

echo "==> Copying amalgamation to $DEPS_DIR"
cp open62541.c "$DEPS_DIR/open62541.c"
cp open62541.h "$DEPS_DIR/open62541.h"

echo "==> Generating headers"
# Find the CSVs in the open62541 source tree
NODESET_CSV="$WORK_DIR/open62541/tools/schema/NodeIds.csv"
STATUS_CSV="$WORK_DIR/open62541/tools/schema/StatusCode.csv"

if [ -f "$NODESET_CSV" ] && [ -f "$STATUS_CSV" ]; then
  ruby "$SCRIPT_DIR/generate_headers.rb" "$NODESET_CSV" "$STATUS_CSV"
  echo "==> Headers regenerated"
else
  echo "==> Warning: CSVs not found, skipping header generation"
  echo "   Expected: $NODESET_CSV"
  echo "   Expected: $STATUS_CSV"
fi

echo "==> Done. open62541 updated to $TAG"
