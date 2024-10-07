# Makefile for compiling and running code using leg and gcc
run: 
	gcc -o code code.c

parser:
	./parser/leg -o ./parser/parser.c ./parser/parser.leg
	gcc -c -o ./parser/parser.o ./parser/parser.c

clean:
	rm -f code code.c