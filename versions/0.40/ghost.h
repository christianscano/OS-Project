#ifndef _GHOST_H
#define _GHOST_H

#include "game.h"

/* --- DICHIARAZIONE FUNZIONI GLOBALI --- */

void inizialiseGhosts(gameState* game);
void reinizialiseGhosts(gameState* game);
void updateGhost(gameState* game);
void startGhostsThread(gameState* game);
void freeGhosts(gameState* game);

#endif
