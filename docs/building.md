# Building the game module

This repository now ships with a CMake build that targets the Quake II game DLL/SO located under `src/game/`.

Retail Oblivion itself was released only for Windows. The reconstruction in this repository is what enables equivalent game-module builds for Windows, Linux, and macOS.

The repository version is stored in the top-level `VERSION` file and is used by the nightly packaging workflow.

## Prerequisites

- A C11-capable compiler (tested with GCC 11.4 on Ubuntu 22.04)
- [CMake](https://cmake.org/) 3.16 or newer
- Build tools supported by CMake (for example `ninja` or GNU Make)

## Configure and build (Linux / macOS)

```bash
cmake -S . -B build
cmake --build build
```

The resulting shared object is placed in `build/` with an architecture-specific filename:

- Linux x86: `gamei386.so`
- Linux x64: `gamex86_64.so`
- macOS arm64: `gamearm64.dylib`
- macOS x64: `gamex86_64.dylib`

To build the additional Linux x86 variant on an x64 host:

```bash
sudo apt-get update
sudo apt-get install -y gcc-multilib libc6-dev-i386
cmake -S . -B build-x86 -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-m32 -DCMAKE_SHARED_LINKER_FLAGS=-m32
cmake --build build-x86
```

To force an x64 macOS build on Apple Silicon, configure with:

```bash
cmake -S . -B build-x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=x86_64
cmake --build build-x64
```

## Configure and build (Windows)

```powershell
cmake -S . -B build -A Win32 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The resulting DLL will be emitted as `build/Release/gamex86.dll` for Visual Studio generators, or `build/gamex86.dll` for single-config generators.

To build the additional x64 Windows variant:

```powershell
cmake -S . -B build-x64 -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build-x64 --config Release
```

The x64 Windows build emits `build-x64/Release/gamex86_64.dll` by default. If your target source port expects a different x64 DLL name, set `-DOBLIVION_WINDOWS_OUTPUT_NAME=<name>` when configuring.

### Notes

- The build script automatically includes every `.c` file located in `src/game/` and produces a position-independent shared module named `game`.
- The build system now derives the binary basename from the target platform and architecture so release artifacts stay unambiguous across Windows, Linux, and macOS.
- The Windows build uses the existing `game.def` export definition file so that the exports match the original SDK.
- If `C:/q2Clean` exists, the default Windows build also copies the produced DLL into `C:/q2Clean/oblivion-re` for local testing. Set `-DOBLIVION_ENABLE_LOCAL_DEPLOY=OFF` to disable that behavior explicitly.
- You can change the build directory (`build` in the commands above) to any location you prefer.
- To install the compiled library, run `cmake --install build` and look under `lib/` (Linux/macOS) or `bin/` (Windows) inside the installation prefix.

## Nightly packaging

Nightly packaging scripts are provided for Linux, macOS, and Windows:

```bash
bash scripts/release/nightly-linux.sh x86
bash scripts/release/nightly-linux.sh x64
bash scripts/release/nightly-macos.sh arm64
bash scripts/release/nightly-macos.sh x64
```

```powershell
powershell -ExecutionPolicy Bypass -File scripts/release/nightly-windows.ps1 -Arch x86
powershell -ExecutionPolicy Bypass -File scripts/release/nightly-windows.ps1 -Arch x64
```

Each script:

- reads the base semantic version from `VERSION`
- builds a release binary for its requested platform/architecture pair
- stages an `oblivion/` folder containing the platform binary, `pack/oblivion.cfg`, and `README.html`
- writes a versioned archive under `dist/` (`.tar.gz` on Linux/macOS, `.zip` on Windows) with names such as `oblivion-linux-x86-v1.0.0-nightly.20260316.tar.gz`

The GitHub Actions workflow at `.github/workflows/nightly-release.yml` runs these scripts on a nightly schedule and publishes Linux x86/x64, macOS arm64/x64, and Windows x86/x64 archives under a release tagged like `v1.0.0-nightly.20260316`. Manual workflow dispatches use a UTC timestamped tag such as `v1.0.0-nightly.20260316.205724` so same-day manual runs do not overwrite the scheduled nightly release.

These release archives are intended to be extracted over an existing Oblivion installation. They replace the platform game module and `oblivion.cfg`; they do not replace the original mod data install.

## Validation

The Linux instructions above were validated on Ubuntu 22.04 using GCC 11.4 and CMake 3.22.1. The build completed successfully and produced `build/game.so`.
