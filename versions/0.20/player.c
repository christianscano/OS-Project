#include <curses.h>
#include <stdbool.h>
#include <unistd.h>
#include "render.h"
#include "player.h"
#include "area.h"
#include "game.h"
#include "missil.h"

/* --- IMPLEMENTAZIONE FUNZIONI --- */

//Inizializza i valori dell'oggetto pacman
void inizialisePlayer(pacman_object* pacman) {
    pacman->iFrame = 0; //Frame animazione iniziale
    pacman->lastDirValid = SINISTRA; //Ultima direzione valida DEFAULT: sinistra
    pacman->dirValid = SINISTRA; //Ultima direzione valida DEFAULT: sinistra 
    pacman->lives = 3; //Vite iniziali
    pacman->p_pos.x = 26; //Coordinate x inizlae
    pacman->p_pos.y = 23; //Coordinata y iniziale
    pacman->score = 0; //Punteggio iniziale
    pacman->nextGhostScore = 200; //Moltiplicatore fantasmi
}

//Funzione Thread Player
void* playerRoutine(void* param) {
    pacman_object* pacman = (pacman_object*) param;
    int input;

    while(pacman->lives > 0) { 
        input = getch();

        if(input == SU || input == GIU || input == SINISTRA || input == DESTRA || input == SPACE)
            if(input == SPACE && pacman->isSpare != true) 
                pacman->isSpare = true;
            else if(input != pacman->lastDirValid && input != SPACE)
                pacman->dirValid = input;
    }

    pthread_exit(NULL);
}

//Funzione che si occupa di aggiornare le coordinate e controllare le collisioni di pacman
void updatePlayer(gameState* game) {
    int x = game->pacman.p_pos.x;
    int y = game->pacman.p_pos.y;
    int tmpDir;

    pthread_mutex_lock(&mtx);
    setSquareMap(game->pacman.p_pos.y, game->pacman.p_pos.x, ' ');
    pthread_mutex_unlock(&mtx);

    if(++game->pacman.iFrame == 6)
        game->pacman.iFrame = 0;
    
    //Carico la nuova posizione inserita in input
    tmpDir = game->pacman.dirValid;
    switch(tmpDir) {
        case SU:
            y -= 1;
            break;
        case GIU:
            y += 1;
            break;
        case SINISTRA:
            x -= 2;
            break;
        case DESTRA:
            x += 2;
            break;
    }

    //Se la nuova posizione non è attualmente valida carico l'ultima valida
    pthread_mutex_lock(&mtx);
    if(!isValidSquare(y, x)) { 
        x = game->pacman.p_pos.x;
        y = game->pacman.p_pos.y;
        tmpDir = game->pacman.lastDirValid;
        switch(tmpDir) {
            case SU:
            y -= 1;
            break;
        case GIU:
            y += 1;
            break;
        case SINISTRA:
            x -= 2;
            break;
        case DESTRA:
            x += 2;
            break;
        }
    }
    
    if(x >= 0 && x < MAXX) {
        if(isValidSquare(y, x)) {
            game->pacman.lastDirValid = tmpDir;
            game->pacman.p_pos.x = x;
            game->pacman.p_pos.y = y;

            if(getSquareMap(y, x) == '~' || getSquareMap(y, x) == '*') {
                setSquareMap(y, x, ' ');
                game->pill--;
                game->pacman.score += 10;
            }
        }
    } else if(x < 0) { //Tunnel
        game->pacman.p_pos.x = MAXX-1;
    } else if(x >= MAXX)
        game->pacman.p_pos.x = 0;
    pthread_mutex_unlock(&mtx);
    
    //Controlla se il giocatore a premuto il tasto SPACE generando quattro spari
    if(game->pacman.isSpare) { 
        pthread_mutex_lock(&mtx);
        loadMissil(game);
        pthread_mutex_unlock(&mtx);
        game->pacman.isSpare = false;
    }
}
