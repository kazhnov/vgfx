DEBUG = -g -O0 -fsanitize=address -Wall -Wextra -Wno-unused-parameter -Werror -Wno-error=cpp
RELEASE = -O3
MODE = $(DEBUG)
nix:
	nix develop --extra-experimental-features nix-command --extra-experimental-features flakes

subs:
	mkdir -p lib
	git submodule update --init --recursive
	git submodule foreach --recursive git pull origin main
	(cd include/vmesh && make build -B)
	(cd include/vtex && make build -B)

build: vgfx.c
	mkdir -p include build lib build/examples
	cp vgfx.h include/
	cc -fPIC -I./include/ -c vgfx.c -o build/vgfx.o $(MODE)
	cc -fPIC -I./include/ -c glad.c -o build/glad.o $(MODE)
	ar rc lib/libvgfx.a build/*.o include/vmesh/build/*.o include/vtex/build/*.o

clear: example/clear_screen.c build
	cc example/clear_screen.c -o build/examples/clear -L./lib -lm -lvgfx -lglfw $(MODE)
	build/examples/clear

shapes: example/shapes.c build
	cc example/shapes.c -o build/examples/shapes -L./lib -lvgfx -lm -lglfw $(MODE)
	build/examples/shapes

mesh: example/mesh.c build
	cc example/mesh.c -o build/examples/mesh -L./lib -lvgfx -lm -lglfw $(MODE)
	build/examples/mesh
