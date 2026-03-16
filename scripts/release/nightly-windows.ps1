param(
    [ValidateSet('x86', 'x64')]
    [string]$Arch = $(if ($env:OBLIVION_ARCH) { $env:OBLIVION_ARCH } else { 'x86' })
)

$ErrorActionPreference = 'Stop'

$rootDir = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$versionFile = Join-Path $rootDir 'VERSION'
$moduleName = if ($env:OBLIVION_WINDOWS_OUTPUT_NAME) {
    $env:OBLIVION_WINDOWS_OUTPUT_NAME
} elseif ($Arch -eq 'x64') {
    'gamex86_64'
} else {
    'gamex86'
}
$generatorPlatform = if ($Arch -eq 'x64') { 'x64' } else { 'Win32' }

if (-not (Test-Path $versionFile)) {
    throw 'Missing VERSION file.'
}

$baseVersion = (Get-Content $versionFile -Raw).Trim()
if ($baseVersion -notmatch '^[0-9]+\.[0-9]+\.[0-9]+$') {
    throw 'VERSION must contain semantic version text like 0.1.0'
}

$nightlyStamp = if ($env:NIGHTLY_STAMP) { $env:NIGHTLY_STAMP } else { (Get-Date).ToUniversalTime().ToString('yyyyMMdd') }
$releaseTag = if ($env:RELEASE_TAG) { $env:RELEASE_TAG } else { "v$baseVersion-nightly.$nightlyStamp" }
$buildDir = if ($env:BUILD_DIR) { $env:BUILD_DIR } else { Join-Path $rootDir "build-nightly-windows-$Arch" }
$distRoot = if ($env:DIST_DIR) { $env:DIST_DIR } else { Join-Path $rootDir "dist\windows-$Arch" }
$stageDir = Join-Path $distRoot 'oblivion'
$archiveName = "oblivion-windows-$Arch-$releaseTag.zip"
$archivePath = Join-Path (Join-Path $rootDir 'dist') $archiveName

cmake -S $rootDir -B $buildDir -A $generatorPlatform -DCMAKE_BUILD_TYPE=Release -DOBLIVION_ENABLE_LOCAL_DEPLOY=OFF "-DOBLIVION_TARGET_ARCH=$Arch" "-DOBLIVION_WINDOWS_OUTPUT_NAME=$moduleName"
cmake --build $buildDir --config Release

$candidates = @(
    (Join-Path $buildDir "Release\$moduleName.dll"),
    (Join-Path $buildDir "$moduleName.dll")
)

$binaryPath = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $binaryPath) {
    throw "Unable to find built binary $moduleName.dll under $buildDir"
}

if (Test-Path $distRoot) {
    Remove-Item $distRoot -Recurse -Force
}

New-Item -ItemType Directory -Path $stageDir -Force | Out-Null
Copy-Item $binaryPath (Join-Path $stageDir "$moduleName.dll") -Force
Copy-Item (Join-Path $rootDir 'pack\oblivion.cfg') (Join-Path $stageDir 'oblivion.cfg') -Force
Copy-Item (Join-Path $rootDir 'docs\release-readme.html') (Join-Path $stageDir 'README.html') -Force

$archiveDir = Join-Path $rootDir 'dist'
New-Item -ItemType Directory -Path $archiveDir -Force | Out-Null
if (Test-Path $archivePath) {
    Remove-Item $archivePath -Force
}

Compress-Archive -Path (Join-Path $distRoot 'oblivion') -DestinationPath $archivePath -CompressionLevel Optimal

if ($env:GITHUB_OUTPUT) {
    Add-Content -Path $env:GITHUB_OUTPUT -Value "archive_path=$archivePath"
    Add-Content -Path $env:GITHUB_OUTPUT -Value "archive_name=$archiveName"
    Add-Content -Path $env:GITHUB_OUTPUT -Value "release_tag=$releaseTag"
    Add-Content -Path $env:GITHUB_OUTPUT -Value "base_version=$baseVersion"
    Add-Content -Path $env:GITHUB_OUTPUT -Value "arch=$Arch"
}

Write-Host "Created $archivePath"
