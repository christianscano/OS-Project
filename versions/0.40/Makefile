pacman.out: pacman.o game.o player.o render.o area.o missil.o ghost.o menu.o
	gcc -o pacman pacman.o game.o area.o render.o player.o missil.o ghost.o menu.o -lncurses -pthread

game.o: game.h game.c
	gcc -c game.c -pthread

area.o: area.h area.c
	gcc -c area.c

render.o: render.h render.c
	gcc -c render.c

player.o: player.h player.c
	gcc -c player.c -pthread

pacman.o: pacman.h pacman.c
	gcc -c pacman.c 

missil.o: missil.h missil.c 
	gcc -c missil.c -pthread

ghost.o: ghost.h ghost.c
	gcc -c ghost.c -pthread

menu.o: menu.h menu.c
	gcc -c menu.c

clean:
	rm -f *.o
