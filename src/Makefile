# Makefile for compiling and running code using leg and gcc

parser:
	./parser/leg -o ./parser/parser.c ./parser/parser.leg
	gcc -c -o ./parser/parser.o ./parser/parser.c

run: 
	gcc -o code code.c

clean:
	rm -f code code.c