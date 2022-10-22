#ifndef _PLAYER_H
#define _PLAYER_H

#include "game.h"

/* --- DICHIARAZIONE FUNZIONI GLOBALI --- */

void* playerRoutine(void* param);
void updatePlayer(gameState* game);
void inizialisePlayer(pacman_object* pacman);
void reinizialisePlayer(pacman_object* pacman);
void pacmanVsGhost(ghost_object* ghostArray, pacman_object* pacman);

#endif