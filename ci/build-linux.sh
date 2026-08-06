#!/bin/bash
GITROOT=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/.." &> /dev/null && pwd )
cfg="${1:-RelWithDebInfo}"

# Stop the script when a native command fails
set -xe -o pipefail

export CMAKE_GENERATOR="Ninja Multi-Config"
export VERBOSE=1

cd -- "$GITROOT"
cmake -B ./build \
	-DCMAKE_C_COMPILER="gcc" \
	-DCMAKE_CXX_COMPILER="g++" \
	-DCMAKE_C_FLAGS="-march=x86-64-v3" \
	-DCMAKE_CXX_FLAGS="-march=x86-64-v3" \
	-S .

cmake --build ./build --config $cfg

pushd ./build
ctest --build-config $cfg
popd

cmake --install ./build --config $cfg --prefix "$GITROOT/install/$cfg"
