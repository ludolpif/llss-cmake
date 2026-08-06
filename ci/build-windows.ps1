param (
	[Parameter(Mandatory = $false)]
	[string]$cfg
)

$GITROOT = "$PSScriptRoot/.."
if (!$PSBoundParameters.ContainsKey('cfg')) {
	$cfg = "RelWithDebInfo"
}
if ($cfg -ne "Debug" -and $cfg -ne "RelWithDebInfo" -and $cfg -ne "Release") {
	[Console]::Error.WriteLine("Usage: $PSCommandPath (Debug|RelWithDebInfo|Release)")
	exit 1
}

# Stop the script when a native command or a cmdlet fails
$PSNativeCommandUseErrorActionPreference = $true
$ErrorActionPreference = 'Stop'

"::notice $env:CMAKE_GENERATOR = Visual Studio 18 2026"
$env:CMAKE_GENERATOR = "Visual Studio 18 2026"
"::notice $env:VERBOSE = 1"
$env:VERBOSE = 1

"::notice cd $GITROOT"
cd $GITROOT

# powershell "&" is call operator, not unix background task meta-character

'::notice cmake -B ./build -D[...] -S .'
& cmake -B ./build `
	-DCMAKE_C_COMPILER="cl" `
	-DCMAKE_CXX_COMPILER="cl" `
	-DCMAKE_C_FLAGS="/arch:AVX2" `
	-DCMAKE_CXX_FLAGS="/arch:AVX2" `
	-S .

"::notice cmake --build ./build --config $cfg"
& cmake --build ./build --config $cfg

"::notice pushd ./build"
pushd ./build
"::notice ctest --build-config $cfg"
& ctest --build-config $cfg
"::notice popd"
popd

"::notice cmake --install build --config $cfg --prefix $GITROOT/install/$cfg"
& cmake --install ./build --config $cfg --prefix "$GITROOT/install/$cfg"
