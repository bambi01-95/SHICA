leg: 
	leg -o ./compiler/parser/parser.c  ./compiler/parser/parser.leg

com:
	gcc -o shica ./compiler.c

all:
	gcc -o vm executor.c
	./vm

vms:
	gcc -o vm executor.c

	
run:
	./vm


pi:
	gcc -o vm executor.c -lpigpio -lpthread

gitShare:
	git add .
	git commit -m "update"
	git push

gitUpdata:
	git fetch origin
	git pull origin main

daygit:
	git add .
	git commit -m "dalily commit"
	git push

gitKillUpdata:
	git reset --hard
	git fetch origin
	git reset --hard origin/main