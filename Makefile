CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,output/%.o,$(SRCS))

mini_compiler: $(OBJS)
		cc $(CFLAGS) -o output/mini_compiler $(OBJS)

output/%.o: %.c
		cc $(CFLAGS) -c $< -o $@

output/util.o: util.h
output/tokenizer.o: tokenizer.h util.h
output/parser.o: parser.h tokenizer.h
output/codegen.o: codegen.h parser.h
output/main.o: util.h tokenizer.h parser.h codegen.h

test: output/mini_compiler
		./test.sh

clean: 
		rm -f output/mini_compiler output/*.o *~ output/tmp*

.PHONY: test clean