.PHONY: dev clean

dev:
	VERBOSE=1 cmake --preset=default -DCMAKE_C_FLAGS="-Wall -march=x86-64-v3" -DCMAKE_CXX_FLAGS="-Wall -march=x86-64-v3"
	VERBOSE=1 cmake --build --preset=default --config Debug
	VERBOSE=1 cmake --install build --config Debug --prefix install/Debug
clean:
	cmake --build --preset=default --config Debug --target=clean
	rm -vf install/Debug/bin/llss

