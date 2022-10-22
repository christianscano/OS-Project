#ifndef _MISSIL_H
#define _MISSIL_H

#include "game.h"

/* --- DICHIARAZIONE FUNZIONI GLOBALI --- */

void* missilRoutine(void* param);
void inizialiseArrayMissil(listMissil* missilArray);
void reinizialiseArrayMissil(listMissil* missilArray, char map[MAXY][MAXX]);
void updateMissils(gameState* game);
void loadMissil(gameState* game, bool isPacman, int index);
void missilVsGhostVsPacman(ghost_object* ghostArray, pacman_object* pacman, listMissil* list, char map[MAXY][MAXX]);

#endif