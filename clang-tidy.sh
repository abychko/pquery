#!/usr/bin/env bash

OPTS=""
EXTRA_ARGS=()

if [[ $# -gt 1 ]]; then
  echo "Usage: $0 [--fix]"
  exit 1
fi

if [[ $# -eq 1 ]]; then
  if [[ "$1" == "--fix" ]]; then
    OPTS="-fix"
  else
    echo "Usage: $0 [--fix]"
    exit 1
  fi
fi

if ! command -v clang-tidy >/dev/null 2>&1; then
  echo "* clang-tidy is not installed!"
  exit 1
fi

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd) || exit 1
cd "$script_dir" || exit 1

if [[ ! -f ./compile_commands.json ]]; then
  echo "* compile_commands.json not found in project root!"
  echo "* Generate it first, for example with CMake:"
  echo "  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ."
  exit 1
fi

case "$(uname -s)" in
  Darwin)
    if ! command -v xcrun >/dev/null 2>&1; then
      echo "* xcrun is not available!"
      exit 1
    fi
    EXTRA_ARGS+=(-- -isysroot "$(xcrun --show-sdk-path)" -I./src)
    ;;
esac

find . \
  \( -path './.git' -o -path './build' \) -prune -o \
  -type f \( -name '*.c' -o -name '*.cpp' \) -print |
while IFS= read -r file; do
  echo "- checking $file"
  clang-tidy "$file" -p . $OPTS "${EXTRA_ARGS[@]}"
done
