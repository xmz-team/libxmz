#!/usr/bin/env bash
set -uo pipefail

ROOT=$(cd $(dirname $0)/.. && pwd)/../..
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

mkdir -p "$WORK/inc"
ln -s "$ROOT" "$WORK/inc/libxmz"

CXX="${CXX:-c++}"
CXXFLAGS="${CXXFLAGS:--std=c++20 -Wall -Wextra -O0 -g}"

TESTS=(
  test.cc
  test-io.cc
  test-log.cc
  test-aux.cc
  test-fs.cc
  test-fs2.cc
  test-str.cc
  test-cp.cc
  test-runsh.cc
  test-io-no-auto-space.cc
  test-io-auto-space.cc
)

fail=0
for t in "${TESTS[@]}"
do
  name="${t%.cc}"
  out="$WORK/$name"
  echo "=== build $t ==="
  if ! $CXX $CXXFLAGS -I"$WORK/inc" "$ROOT/Tests/$t" -o "$out"; then
    echo "BUILD FAIL: $t"
    fail=1
    continue
  fi
  echo "=== run $t ==="
  rundir="$WORK/run-$name"
  mkdir -p "$rundir"
  ( cd "$rundir" && "$out" ) || { echo "RUN FAIL: $t"; fail=1; }
done

exit $fail
