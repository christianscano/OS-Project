pacman.out: pacman.o game.o player.o render.o area.o missil.o ghost.o
	gcc pacman.o game.o area.o render.o player.o missil.o ghost.o -lncurses -pthread

game.o: game.h game.c
	gcc -c game.c -lncurses -pthread

area.o: area.h area.c
	gcc -c area.c -lncurses -pthread

render.o: render.h render.c
	gcc -c render.c -lcurses -pthread

player.o: player.h player.c
	gcc -c player.c -lncurses -pthread

pacman.o: pacman.h pacman.c
	gcc -c pacman.c 

missil.o: missil.h missil.c 
	gcc -c missil.c -lncurses -pthread

ghost.o: ghost.h ghost.c
	gcc -c ghost.c -pthread

clean:
	rm -f *.o
