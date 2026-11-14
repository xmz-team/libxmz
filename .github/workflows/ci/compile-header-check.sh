#!/usr/bin/env bash
set -uo pipefail
ROOT=$(cd $(dirname $0)/.. && pwd)
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK/inc"
ln -s "$ROOT" "$WORK/inc/libxmz"

# platform exclusive has been excluded
HEADERS=(
  io.hpp str.hpp time.hpp aux.hpp fs.hpp _fs.hpp
  crash.hpp log.hpp runsh.hpp
)

fail=0
for h in "${HEADERS[@]}"
do
  tu="$WORK/tu-$h.cpp"
  printf '#include <libxmz/%s>\nint main(){return 0;}\n' "$h" > "$tu"
  echo "=== self-contain: $h ==="
  if ! ${CXX:-c++} -std=c++20 -Wall -Wextra -fsyntax-only -I"$WORK/inc" "$tu"; then
    echo "SELF-CONTAIN FAIL: $h"
    fail=1
  fi
done
exit $fail
