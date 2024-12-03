leg: 
	./compiler/parser/leg -o ./compiler/parser/parser.c  ./compiler/parser/parser.leg

com:
	gcc -o shica ./compiler.c



all:
	gcc -o vm executor.c
	./vm

vm:
	gcc -o vm executor.c

run:
	./vm