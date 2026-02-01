subs:
	git submodule foreach git pull origin main

build: vgfx.c
	mkdir -p include build lib build/examples
	cp vgfx.h include/
	cc vgfx.c -fPIC -c -o build/vgfx.o
	cc glad.c -fPIC -c -o build/glad.o
	cc -shared -o lib/libvgfx.so build/glad.o build/vgfx.o -lm -lglfw

clear: example/clear_screen.c build
	cc example/clear_screen.c -o build/examples/clear -L./lib -lvgfx
	LD_LIBRARY_PATH=./lib build/examples/clear

shapes: example/shapes.c build
	cc example/shapes.c -o build/examples/shapes -L./lib -lvgfx
	LD_LIBRARY_PATH=./lib build/examples/shapes
