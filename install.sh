#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "$0")" && pwd)"

python_bin="${PYTHON:-}"
if [[ -z "$python_bin" ]]; then
  if command -v python3 >/dev/null 2>&1; then
    python_bin="$(command -v python3)"
  elif command -v python >/dev/null 2>&1; then
    python_bin="$(command -v python)"
  fi
fi

if [[ -z "$python_bin" ]]; then
  printf '%s\n' 'Python 3 is required to run this installer.' >&2
  exit 1
fi

exec "$python_bin" "$script_dir/scripts/install.py" "$script_dir"
