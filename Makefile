# Main build system (generator) is CMake
# This file is a convinent helper to have :make <something> commands for vim devs
.PHONY: dev run clean

dev:
	scripts/build-linux.sh Debug
run:
	install/Debug/bin/llss
clean:
	cmake --build ./build --config Debug --target=clean
	rm -rf install/Debug

