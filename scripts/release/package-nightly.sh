#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
	echo "usage: $0 <linux|macos> <binary-name> [arch]" >&2
	exit 1
fi

platform="$1"
binary_name="$2"
arch="${3:-${OBLIVION_ARCH:-}}"
root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
version_file="$root_dir/VERSION"

if [[ -z "$arch" ]]; then
	case "$(uname -m)" in
		x86_64|amd64)
			arch="x64"
			;;
		arm64|aarch64)
			arch="arm64"
			;;
		i?86)
			arch="x86"
			;;
		*)
			echo "unable to infer architecture from $(uname -m); pass [arch] explicitly" >&2
			exit 1
			;;
	esac
fi

if [[ ! -f "$version_file" ]]; then
	echo "missing VERSION file" >&2
	exit 1
fi

base_version="$(tr -d '\r\n' < "$version_file")"
if [[ ! "$base_version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
	echo "VERSION must contain semantic version text like 0.1.0" >&2
	exit 1
fi

nightly_stamp="${NIGHTLY_STAMP:-$(date -u +%Y%m%d)}"
release_tag="${RELEASE_TAG:-v${base_version}-nightly.${nightly_stamp}}"
build_dir="${BUILD_DIR:-$root_dir/build-nightly-$platform-$arch}"
dist_dir="${DIST_DIR:-$root_dir/dist/$platform-$arch}"
stage_dir="$dist_dir/oblivion"
archive_name="oblivion-${platform}-${arch}-${release_tag}.tar.gz"
archive_path="$root_dir/dist/$archive_name"

configure_args=(
	-DCMAKE_BUILD_TYPE=Release
)

case "$platform" in
	linux)
		if [[ "$arch" != "x64" ]]; then
			echo "linux nightly packaging currently supports only x64" >&2
			exit 1
		fi
		;;
	macos)
		case "$arch" in
			x64)
				configure_args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
				;;
			arm64)
				configure_args+=(-DCMAKE_OSX_ARCHITECTURES=arm64)
				;;
			*)
				echo "macos nightly packaging supports x64 or arm64" >&2
				exit 1
				;;
		esac
		;;
	*)
		echo "unsupported platform '$platform'" >&2
		exit 1
		;;
esac

cmake -S "$root_dir" -B "$build_dir" "${configure_args[@]}"
cmake --build "$build_dir" --config Release

binary_path=""
for candidate in \
	"$build_dir/$binary_name" \
	"$build_dir/Release/$binary_name"
do
	if [[ -f "$candidate" ]]; then
		binary_path="$candidate"
		break
	fi
done

if [[ -z "$binary_path" ]]; then
	echo "unable to find built binary $binary_name under $build_dir" >&2
	exit 1
fi

rm -rf "$dist_dir"
mkdir -p "$stage_dir"
cp "$binary_path" "$stage_dir/$binary_name"
cp "$root_dir/pack/oblivion.cfg" "$stage_dir/oblivion.cfg"
cp "$root_dir/docs/release-readme.html" "$stage_dir/README.html"

mkdir -p "$root_dir/dist"
tar -C "$dist_dir" -czf "$archive_path" oblivion

if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
	{
		echo "archive_path=$archive_path"
		echo "archive_name=$archive_name"
		echo "release_tag=$release_tag"
		echo "base_version=$base_version"
		echo "arch=$arch"
	} >> "$GITHUB_OUTPUT"
fi

echo "created $archive_path"
