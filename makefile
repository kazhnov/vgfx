subs:
	git submodule foreach git pull origin main

build: vgfx.c
	cc vgfx.c -fPIC -c -o vgfx.o
	cc -shared -o libvgfx.so vgfx.o -lSDL3
