#include <pthread.h>
#include <curses.h>
#include <unistd.h>
#include "render.h"
#include "player.h"
#include "area.h"
#include "game.h"
#include "ghost.h"
#include "missil.h"

/* --- DICHIARAZIONE FUNZIONI INTERNE --- */
void gameLoop(gameState* game);
void inizialiseGame(gameState* game);
void drawAll(gameState* game);

/* --- IMPLEMENTAZIONE FUNZIONE --- */ 
void mainGame() {
    gameState game;
    configCurses();
    
    pthread_mutex_init(&rw_mutex, NULL);

    inizialiseGame(&game);
    
    //Attesa necessaria
    usleep(1000);  

    do {
        gameLoop(&game);
        switch(game.status) {
            case PACMAN_DIE:
                game.pacman.lives--;

                if(game.pacman.lives < 0)
                    game.status = END_GAME;

                //Si rinizializza tutto
                break;

            case COMPLETE_GAME:
                break;
        }

    } while(game.status != END_GAME);

    endwin();
}

//
void inizialiseGame(gameState* game) {
    game->pill = NUM_PILL;
    
    repopulateMap(game);
    inizialisePlayer(&game->pacman);
    inizialiseArrayMissil(game);
    inizialiseGhosts(game);
}

//
void gameLoop(gameState* game) {
    drawAll(game);
    refresh();
    usleep(3000000);

    //Inizio Gioco
    while(game->pill > 0) {
        pthread_mutex_lock(&rw_mutex);
        
        updateMissils(game);
        updatePlayer(game);
        updateGhost(game);

        drawAll(game);

        pthread_mutex_unlock(&rw_mutex);

        usleep(95000);
    }
}

//Disegna tutti gli oggetti a schermo
void drawAll(gameState* game) {
    drawMap(game->map);
    drawPlayer(game->pacman);
    drawGhosts(game->ghostArray);
    mvprintw(33,0, "Score: %d", game->pacman.score);

    int i;
    for(i = 0; i < NUM_GHOST; i++)
        mvprintw(i, 57, "id %d DIR: %d", i, game->ghostArray[i].lastDirValid);

    refresh();
}