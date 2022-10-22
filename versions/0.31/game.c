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
statusGame gameLoop(gameState* game);
void inizialiseGame(gameState* game);
void drawAll(gameState* game);

/* --- IMPLEMENTAZIONE FUNZIONE --- */ 
void mainGame() {
    gameState game;
    statusGame status;

    game.mode = HARD;

    configCurses();
    inizialiseGame(&game);
    pthread_mutex_init(&rw_mutex, NULL);
    
    //Attesa necessaria
    usleep(1000);  
    do {
        status = gameLoop(&game);
        switch(status) {
            case PACMAN_DIE:
                pthread_mutex_lock (&rw_mutex);

                game.pacman.lives--;
                if(game.pacman.lives < 0) {
                    status = END_GAME;
                    break;
                }
                
                //Si rinizializza tutto
                reinizialisePlayer(&game.pacman);
                reinizialiseArrayMissil(&game.missilArray, game.map);
                reinizialiseGhosts(&game);
                usleep(1000);

                pthread_mutex_unlock(&rw_mutex);
                break;

            case COMPLETE_GAME:
                break;
        }
    } while(status != USER_QUIT);

    endwin();
}

//
void inizialiseGame(gameState* game) {
    repopulateMap(game);
    inizialisePlayer(&game->pacman);
    inizialiseArrayMissil(&game->missilArray);
    inizialiseGhosts(game);
}

//
statusGame gameLoop(gameState* game) {
    drawAll(game);
    refresh();
    usleep(3000000);
    startGhostsThread(game);

    //Inizio Gioco
    while(true) {
        pthread_mutex_lock(&rw_mutex);
        
        updateMissils(game);
        updatePlayer(game);
        //ghostVsPacman(&game->pacman, game->ghostArray);
        updateGhost(game);
        //ghostVsPacman(&game->pacman, game->ghostArray);
        pacmanVsGhost(game->ghostArray, &game->pacman);
        
        drawAll(game);

        pthread_mutex_unlock(&rw_mutex);

        if(game->pill == 0) 
            return COMPLETE_GAME; 
        else if(game->pacman.isDeath == true)
            return PACMAN_DIE;

        usleep(125000);
    }
}

//Disegna tutti gli oggetti a schermo
void drawAll(gameState* game) {
    drawMap(game->map);
    drawPlayer(game->pacman);
    drawGhosts(game->ghostArray);
    mvprintw(33,0, "Score: %d", game->pacman.score);
    
    /*int i;
    for(i = 0; i < NUM_GHOST; i++)
        mvprintw(i, 57, "id %d DIR: %d x: %.2d y: %.2d", i, game->ghostArray[i].lastDirValid, game->ghostArray[i].p_pos.x, game->ghostArray[i].p_pos.y);

    mvprintw(i, 57, "pacman x: %.2d y: %.2d", game->pacman.p_pos.x, game->pacman.p_pos.y);*/
    refresh();
}