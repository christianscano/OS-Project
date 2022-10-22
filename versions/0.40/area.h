#ifndef _AREA_H
#define _AREA_H

#include "game.h"

/* --- DICHIARAZIONE FUNZIONI GLOBALI --- */

char getSquareMap(char map[MAXY][MAXX], int y, int x);
void setSquareMap(char map[MAXY][MAXX], int y, int x, char c);
bool isValidSquare(char map[MAXY][MAXX], int y, int x);
void repopulateMap(gameState* game);

#endif