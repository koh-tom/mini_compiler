CFLAGS=-std=c11 -g -static

mini_compiler: mini_compiler.c
		cc $(CFLAGS) -o output/mini_compiler mini_compiler.c

test: output/mini_compiler
		./test.sh

clean: 
		rm -f output/mini_compiler *.o *~ output/tmp*

.PHONY: test clean