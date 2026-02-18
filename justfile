build:
	cmake -B build -S . -DCMAKE_INSTALL_PREFIX=install
	cmake --build build
	cmake --install build

format:
	find . \( -name '*.cc' -o -name '*.h' \) -print0 | xargs -0 clang-format -i
