all: leg com vm

leg: 
	leg -o ./compiler/parser/parser.c  ./compiler/parser/parser.leg

com:
	gcc -o shica ./compiler.c


vm:
	gcc -o vm executor.c

	
run:
	./vm

clean:
	rm -f ./vm
	rm -f ./shica

pi:
	gcc -o vm executor.c -lpigpio -lpthread

git:
	rm -f ./vm
	rm -f ./shica
	git add .
	git commit -m "$(if $(MSG),$(MSG),Debug)"
	git push

gitUpdata:
	git fetch origin
	git pull origin main

gitKillUpdata:
	git reset --hard
	git fetch origin
	git reset --hard origin/main

.PHONY: 