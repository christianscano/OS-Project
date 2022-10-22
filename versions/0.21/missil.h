#ifndef _MISSIL_H
#define _MISSIL_H

#include "game.h"

/* --- DICHIARAZIONE FUNZIONI GLOBALI --- */

void* missilRoutine(void* param);
void inizialiseArrayMissil(gameState* game);
void updateMissils(gameState* game);
void loadMissil(gameState* game);

#endif