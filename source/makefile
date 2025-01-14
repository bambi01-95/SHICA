vmleg: 
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

git:
	git fetch origin
	git pull origin main

daygit:
	git add .
	git commit -m "dalily commit"
	git push origin main