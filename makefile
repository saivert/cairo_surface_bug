all: helloCairo

helloCairo: helloCairo.c
	gcc -o helloCairo helloCairo.c `pkgconf --libs --cflags gtk+-3.0`

clean:
	rm helloCairo
