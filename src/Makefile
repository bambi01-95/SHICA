# Makefile for compiling and running code using leg and gcc

run: 
	./leg -o code.c code.leg
	gcc -o code code.c

clean:
	rm -f code code.c

test: 
	echo "implement test code"
	sleep 5
	./code ../test/while-test.txt
	./code