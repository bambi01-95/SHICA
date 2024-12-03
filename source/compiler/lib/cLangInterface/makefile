all:
	./leg -o test.c cParser.leg
	sleep 1
	gcc -o test test.c
	sleep 1
	./test < input.txt

leg:
	./leg -o test.c cParser.leg

gcc:
	gcc -o test test.c

run:
	./test < input.txt