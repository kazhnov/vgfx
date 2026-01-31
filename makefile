subs:
	git submodule foreach git pull origin main

build: vgfx.c
	cp vgfx.h include/
	cc vgfx.c -fPIC -c -o build/vgfx.o
	cc -shared -o lib/libvgfx.so build/vgfx.o -lSDL3
