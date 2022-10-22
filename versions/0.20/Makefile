pacman.out: pacman.o game.o player.o render.o area.o missil.o
	gcc pacman.o game.o area.o render.o player.o missil.o -lncurses -pthread

game.o: missil.h render.h player.h area.h game.h game.c
	gcc -c game.c -lncurses -pthread

area.o: area.h area.c
	gcc -c area.c -lncurses -pthread

render.o: area.h player.h render.h render.c
	gcc -c render.c -lcurses -pthread

player.o: missil.h area.h game.h player.h player.c
	gcc -c player.c -lncurses -pthread

pacman.o: game.h pacman.h pacman.c
	gcc -c pacman.c 

missil.o: game.h area.h missil.h missil.c 
	gcc -c missil.c -lncurses -pthread

clean:
	rm -f *.o