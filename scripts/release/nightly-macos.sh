#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
arch="${1:-${OBLIVION_ARCH:-}}"
case "$arch" in
	x64)
		binary_name="gamex86_64.dylib"
		;;
	arm64)
		binary_name="gamearm64.dylib"
		;;
	*)
		echo "macos nightly packaging supports x64 or arm64" >&2
		exit 1
		;;
esac
bash "$script_dir/package-nightly.sh" macos "$binary_name" "$arch"
