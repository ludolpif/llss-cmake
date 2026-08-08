param (
	[Parameter(Mandatory = $false)]	[string]$cfg
)
# This script could be double-clicked from explorer.exe and really inform user without disapearing on first error,
# or called from a shell, or called from a CI and not hang for an infinite amount of time.

$GITROOT = "$PSScriptRoot/.."
if (!$PSBoundParameters.ContainsKey('cfg')) {
	$cfg = "RelWithDebInfo"
}
if ($cfg -ne "Debug" -and $cfg -ne "MinSizeRel" -and $cfg -ne "RelWithDebInfo" -and $cfg -ne "Release") {
	Write-Error "Usage: $PSCommandPath (Debug|MinSizeRel|RelWithDebInfo|Release)"
	exit 1
}
if (-not $env:SDL3_DIR) {
	Write-Warning "cmake may need SDL3_DIR to be set to folder of the unziped SDL3-devel-*-VC.zip"
}
if (-not $env:SDL3_image_DIR) {
	Write-Warning "cmake may need SDL3_image_DIR to be set to folder of the unziped SDL3_image-devel-*-VC.zip"
}
if (-not $env:SDL3_ttf_DIR) {
	Write-Warning "cmake may need SDL3_ttf_DIR to be set to folder of the unziped SDL3_ttf-devel-*-VC.zip"
}

'::notice $env:CMAKE_GENERATOR = Visual Studio 18 2026'
$env:CMAKE_GENERATOR = "Visual Studio 18 2026"
'::notice $env:VERBOSE = 1'
$env:VERBOSE = 1

"::notice cd $GITROOT"
cd $GITROOT

# If set to "Stop", CMake error messages aren't displayed correctly, and output to stderr will halt the script even if exit code is 0
# I can't find an elegant way to have equivalent of "set -e" from bash, after more than one hour, it's very very very anoying.
$ErrorActionPreference = "Continue"

# powershell "&" is call operator, not unix background task meta-character

'::notice cmake -B ./build -D[...] -S .'
& cmake -B ./build `
	-DCMAKE_C_COMPILER="cl" `
	-DCMAKE_CXX_COMPILER="cl" `
	-DCMAKE_C_FLAGS="/arch:AVX2" `
	-DCMAKE_CXX_FLAGS="/arch:AVX2" `
	-S .
if ($LASTEXITCODE -ne 0) { Write-Error "LASTEXITCODE: $LASTEXITCODE."; sleep 10; throw }

"::notice cmake --build ./build --config $cfg"
& cmake --build ./build --config $cfg
if ($LASTEXITCODE -ne 0) { Write-Error "LASTEXITCODE: $LASTEXITCODE."; sleep 10; throw }

"::notice pushd ./build"
pushd ./build
"::notice ctest --build-config $cfg"
& ctest --build-config $cfg
if ($LASTEXITCODE -ne 0) { Write-Error "LASTEXITCODE: $LASTEXITCODE."; sleep 10; throw }
"::notice popd"
popd

"::notice cmake --install build --config $cfg --prefix $GITROOT/install/$cfg"
& cmake --install ./build --config $cfg --prefix "$GITROOT/install/$cfg"
if ($LASTEXITCODE -ne 0) { Write-Error "LASTEXITCODE: $LASTEXITCODE."; sleep 10; throw }

'::notice VisualStudio Solution file is in ./build, resulting executable is in ./install\bin'
'::notice Script ran successfully.'
sleep 10
