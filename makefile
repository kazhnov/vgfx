subs:
	git submodule foreach git pull origin main

build: vgfx.c
	mkdir -p include build lib
	cp vgfx.h include/
	cc vgfx.c -fPIC -c -o build/vgfx.o
	cc -shared -o lib/libvgfx.so build/vgfx.o -lm -lGL -lSDL2

clear: example/clear_screen.c build
	cc example/clear_screen.c -o build/example.o -L./lib -lvgfx
