#include <pthread.h>
#include <curses.h>
#include <unistd.h>
#include "render.h"
#include "player.h"
#include "area.h"
#include "game.h"

void gameLoop(gameState* game);
void inizialiseGame(gameState* game);
void drawAll(gameState* game);

pthread_mutex_t mtx;

void mainGame() {
    pthread_t idPlayer;
    
    gameState game;
    configCurses();

    inizialiseGame(&game);

    pthread_create(&idPlayer, NULL, &playerRoutine, (void*)&game.pacman);
    pthread_mutex_init(&mtx, NULL);

    gameLoop(&game);

    pthread_join(idPlayer, NULL);

    endwin();
}

void inizialiseGame(gameState* game) {
    game->pill = MAXPILL;
    inizialisePlayer(&game->pacman);
    //drawMap();
    //Inizializza fantasmi e altro
}

void gameLoop(gameState* game) {
    drawAll(game);
    refresh();
    sleep(2);

    while(game->pill > 0) {
        updatePlayer(game);
        
        drawAll(game);
        
        refresh();
        usleep(150000);
    }
}

//Disegna tutti gli oggetti a schermo
void drawAll(gameState* game) {
    drawMap();
    usleep(1000);
    drawPlayer(&game->pacman);
    usleep(1000);
    mvprintw(33,0, "Score: %d", game->pacman.score);

}