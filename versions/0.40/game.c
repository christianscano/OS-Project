#include <pthread.h>
#include <curses.h>
#include <unistd.h>
#include <mm_malloc.h>
#include "render.h"
#include "player.h"
#include "area.h"
#include "game.h"
#include "ghost.h"
#include "missil.h"
#include "menu.h"

/* --- DICHIARAZIONE FUNZIONI INTERNE --- */
statusGame gameLoop(gameState* game);
void inizialiseGame(gameState* game);
void drawAll(gameState* game);

/* --- IMPLEMENTAZIONE FUNZIONI --- */ 

//Funzione di avvio principale
void mainGame() {
    gameState game;
    statusGame status;

    configCurses();

    while(loadMenu(&game) == 0) {
        erase();
        inizialiseGame(&game);
        //Attesa necessaria
        usleep(10000);  

        do {
            status = gameLoop(&game);
            switch (status) {
                case PACMAN_DIE:
                    usleep(10000);
                    pthread_mutex_lock(&rw_mutex);
                    game.pacman.lives--;
                    drawPlayerDead(game.pacman);
                    pthread_mutex_unlock(&rw_mutex);
                    //Chiusura thread e reinizializzazione
                    reinizialiseArrayMissil(&game.missilArray, game.map);
                    reinizialiseGhosts(&game);
                    usleep(5000);
                    //Uscita in caso di vite uguali a zero
                    if(game.pacman.lives == 0) {
                        status = END_GAME;
                        drawGameOverScreen();
                        break;
                    }
                    reinizialisePlayer(&game.pacman);
                    usleep(20000);
                    break;

                case COMPLETE_GAME:
                    //Chiusura thread
                    game.pacman.lives = 0;
                    reinizialiseArrayMissil(&game.missilArray, game.map);
                    freeGhosts(&game);
                    drawWinnerScreen();
                    break;
            }
        } while(status != END_GAME && status != COMPLETE_GAME);
        erase(); //Cancella lo schermo prima di ricaricare il menu di gioco
    }

    pthread_mutex_destroy(&rw_mutex);
    endwin();
}

//Funzione che inizializza tutte le variabili di gioco
void inizialiseGame(gameState* game) {
    //Inizilizzazione semaforo
    pthread_mutex_init(&rw_mutex, NULL);
    //Inizilizzaizone dati
    repopulateMap(game);
    inizialisePlayer(&game->pacman);
    inizialiseArrayMissil(&game->missilArray);
    inizialiseGhosts(game);
}

//Loop di gioco principale
statusGame gameLoop(gameState* game) {
    drawAll(game);
    usleep(3000000); //Tempo di attesa iniziale
    
    //Lancio i thread ghost
    startGhostsThread(game);

    //Inizio Gioco
    while(true) {
        pthread_mutex_lock(&rw_mutex);
        updateMissils(game);
        missilVsGhostVsPacman(game->ghostArray, &game->pacman, &game->missilArray, game->map);
        updatePlayer(game);
        updateGhost(game);
        pacmanVsGhost(game->ghostArray, &game->pacman);
        drawAll(game);
        
        //Punti di uscita
        if(game->pill == 0) {
            pthread_mutex_unlock(&rw_mutex);
            return COMPLETE_GAME;
        } else if(game->pacman.isDead) {
            pthread_mutex_unlock(&rw_mutex);
            return PACMAN_DIE;
        }

        pthread_mutex_unlock(&rw_mutex);
        usleep(130000);
    }
}
