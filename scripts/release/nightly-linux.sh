#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
arch="${1:-${OBLIVION_ARCH:-x64}}"
bash "$script_dir/package-nightly.sh" linux game.so "$arch"
