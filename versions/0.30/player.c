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
    pacman->isSpare = false; //Valore default sparo
    pacman->lives = 3; //Vite iniziali
    pacman->p_pos.x = 26; //Coordinate x inizlae
    pacman->p_pos.y = 23; //Coordinata y iniziale
    pacman->score = 0; //Punteggio iniziale
    pacman->nextGhostScore = 200; //Moltiplicatore fantasmi
    pacman->speed = 1; //Velocità movimento pacman

    //Creazione thread
    pthread_create(&pacman->pacmanID, NULL, playerRoutine, pacman);
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

    if(++game->pacman.iFrame == 6)
        game->pacman.iFrame = 0;
    
    if(game->pacman.speed-- == 0) {
        game->pacman.speed = 1;
        
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
        if(!isValidSquare(game->map, y, x)) { 
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
            if(isValidSquare(game->map, y, x)) {
                game->pacman.lastDirValid = tmpDir;
                game->pacman.p_pos.x = x;
                game->pacman.p_pos.y = y;

                if(getSquareMap(game->map, y, x) == '~') {
                    setSquareMap(game->map, y, x, ' ');
                    game->pill--;
                    game->pacman.score += 10;
                } else if(getSquareMap(game->map, y, x) == '*') {
                    setSquareMap(game->map, y, x, ' ');
                    game->pill--;
                    game->pacman.score += 50;
                }
            }
        } else if(x < 0) { //Tunnel
            game->pacman.p_pos.x = MAXX - 1;
        } else if(x >= MAXX)
            game->pacman.p_pos.x = 0;
    }

    //Controlla se il giocatore a premuto il tasto SPACE generando quattro spari
    if(game->pacman.isSpare) { 
        loadMissil(game);
        game->pacman.isSpare = false;
    }
}
