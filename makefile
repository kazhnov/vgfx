DEBUG = -g -O0 -fsanitize=address -Wall -Wextra -Wno-unused-parameter -Wno-error=cpp
RELEASE = -O3
MODE = $(DEBUG)
PREFIX = LD_LIBRARY_PATH="./lib" LSAN_OPTIONS=print_indirect_leaks=0

subs:
	git submodule foreach git pull origin main
	(cd include/vmesh && make build -B)
	cp include/vmesh/lib/libvmesh.so lib/

build: vgfx.c
	mkdir -p include build lib build/examples
	cp vgfx.h include/
	cc -fPIC -I./include/ -c vgfx.c -o build/vgfx.o $(MODE)
	cc -fPIC -I./include/ -c glad.c -o build/glad.o $(MODE)
	cc -shared -o lib/libvgfx.so build/glad.o build/vgfx.o -lm -lglfw -L./lib -lvmesh -I./include/ $(MODE)

clear: example/clear_screen.c build
	cc example/clear_screen.c -o build/examples/clear -L./lib -lm -lvgfx -lvmesh $(MODE)
	$(PREFIX) build/examples/clear

shapes: example/shapes.c build
	cc example/shapes.c -o build/examples/shapes -L./lib -lvgfx -lm -lvmesh $(MODE)
	$(PREFIX) build/examples/shapes

mesh: example/mesh.c build
	cc example/mesh.c -o build/examples/mesh -L./lib -lvgfx -lm -lvmesh $(MODE)
	$(PREFIX) build/examples/mesh
