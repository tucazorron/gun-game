all: run clear

run:
	gcc -pthread gun-game.c && ./a.out

clear:
	rm a.out