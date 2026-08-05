param (
    [Parameter(Mandatory = $false)]
    [string]$Configuration
)

if (!$PSBoundParameters.ContainsKey('Configuration')) {
    $Configuration = "RelWithDebInfo"
}
if ($Configuration -ne "Debug" -and $Configuration -ne "RelWithDebInfo" -and $Configuration -ne "Release") {
    [Console]::Error.WriteLine("Usage: $PSCommandPath (Debug|RelWithDebInfo|Release)")
    exit 1
}

"::notice Set-Location $PSScriptRoot\.."
pushd "$PSScriptRoot\.."

# powershell "&" is call operator, not unix background task meta-character

'::notice --preset=Windows -DCMAKE_C_FLAGS="/arch:AVX2" -DCMAKE_CXX_FLAGS="/arch:AVX2"'
& cmake --preset=Windows -DCMAKE_C_FLAGS="/arch:AVX2" -DCMAKE_CXX_FLAGS="/arch:AVX2"

"::notice  --build --preset=Windows --config $Configuration"
& cmake --build --preset=Windows --config $Configuration

"::notice cmake --install build --config $Configuration --prefix install/$Configuration"
& cmake --install build --config $Configuration --prefix install/$Configuration

popd