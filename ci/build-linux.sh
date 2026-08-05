#!/bin/bash
ROOT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/.." &> /dev/null && pwd )
Configuration="${1:-RelWithDebInfo}"
set -xe
cd -- "$ROOT_DIR"
VERBOSE=1 cmake --preset=Linux -DCMAKE_C_FLAGS="-march=x86-64-v3" -DCMAKE_CXX_FLAGS="-march=x86-64-v3"
VERBOSE=1 cmake --build --preset=Linux --config $Configuration
VERBOSE=1 cmake --install build --config $Configuration --prefix "$ROOT_DIR/install/$Configuration"
