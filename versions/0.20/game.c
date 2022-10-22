#include <pthread.h>
#include <curses.h>
#include <unistd.h>
#include "render.h"
#include "player.h"
#include "area.h"
#include "game.h"
#include "missil.h"

/* --- DICHIARAZIONE FUNZIONI INTERNE --- */
void gameLoop(gameState* game);
void inizialiseGame(gameState* game);
void drawAll(gameState* game);
pthread_t idPlayer;

/* --- IMPLEMENTAZIONE FUNZIONE --- */ 
void mainGame() {
    
    gameState game;
    configCurses();
    
    pthread_mutex_init(&mtx, NULL);

    inizialiseGame(&game);

    pthread_create(&idPlayer, NULL, &playerRoutine, (void*)&game.pacman);
    
    //Attesa necessaria
    usleep(1000);    
    gameLoop(&game);

    pthread_join(idPlayer, NULL);

    endwin();
}

void inizialiseGame(gameState* game) {
    game->pill = MAXPILL;
    
    inizialisePlayer(&game->pacman);
    inizialiseArrayMissil(game);

    //Inizializza fantasmi e altro
}

void gameLoop(gameState* game) {
    drawAll(game);
    refresh();
    usleep(3000000);

    //Inizio Gioco
    while(game->pill > 0) {
        updateMissils(game);
        updatePlayer(game);

        drawAll(game);
        
        usleep(120000);
    }
}

//Disegna tutti gli oggetti a schermo
void drawAll(gameState* game) {
    pthread_mutex_lock(&mtx);
    
    drawPlayer(&game->pacman);
    drawMap();
    mvprintw(33,0, "Score: %d", game->pacman.score);

    /*missil* it = game->missilArray.top;
    int i = 0;

    //if(it != NULL) {
        while(it != NULL) { 
            mvprintw(i, 57, "Pos %d: x: %d y: %d status: %d dir: %d char: %c isRead: %d isWrite: %d", i, it->data.p_pos.x,
            it->data.p_pos.y, it->data.isTerminated, it->data.missilDir, it->data.c, it->data.isRead, it->data.isWrite);
            i++;
            it = it->next;
        }
    //}
    //else
    //{
    //    mvprintw(0,57, "Vuoto");        
    //}
    */
    refresh();

    pthread_mutex_unlock(&mtx);
}