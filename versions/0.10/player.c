#include <curses.h>
#include <stdbool.h>
#include <unistd.h>
#include "render.h"
#include "player.h"
#include "area.h"
#include "game.h"


void inizialisePlayer(pacman_object* pacman) {
    pacman->iFrame = 0;
    pacman->lastDirValid = SINISTRA;
    pacman->dirValid = SINISTRA;
    pacman->lives = 3;
    pacman->p_pos.x = 13;
    pacman->p_pos.y = 23;
    pacman->score = 0;
    pacman->nextGhostScore = 200;
}

//Funzione Thread Player
void* playerRoutine(void* param) {
    pacman_object* pacman = (pacman_object*) param;
    int input;

    while(TRUE) {
        input = getch();

        if(input == SU || input == GIU || input == SINISTRA || input == DESTRA)
            if(input != pacman->lastDirValid)
                pacman->dirValid = input;
    }
}

void updatePlayer(gameState* game) {
    int x = game->pacman.p_pos.x;
    int y = game->pacman.p_pos.y;
    int tmpDir;
    char c; 

    mvaddch(game->pacman.p_pos.y, game->pacman.p_pos.x, ' ');

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
            x -= 1;
            break;
        case DESTRA:
            x += 1;
            break;
    }

    //Se la nuova posizione non è valida carico l'ultima valida
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
            x -= 1;
            break;
        case DESTRA:
            x += 1;
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
}
