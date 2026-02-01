subs:
	git submodule foreach git pull origin main

build: vgfx.c
	mkdir -p include build lib
	cp vgfx.h include/
	cc vgfx.c -fPIC -c -o build/vgfx.o
	cc -shared -o lib/libvgfx.so build/vgfx.o -lGL -lSDL2
