#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
arch="${1:-${OBLIVION_ARCH:-x64}}"
case "$arch" in
	x86)
		binary_name="gamei386.so"
		;;
	x64)
		binary_name="gamex86_64.so"
		;;
	*)
		echo "linux nightly packaging supports x86 or x64" >&2
		exit 1
		;;
esac
bash "$script_dir/package-nightly.sh" linux "$binary_name" "$arch"
