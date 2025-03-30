git:
	rm -f ./source/vm
	rm -f ./source/shica
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