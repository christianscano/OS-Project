#ifndef _RENDER_H
#define _RENDER_H

#include "game.h"

/* --- DICHIARAZIONE FUNZIONI GLOBALI --- */

void configCurses();
void drawPlayer(pacman_object pacman);
void drawPlayerDead(pacman_object pacman);
void drawGhosts(ghost_object ghost[]);
void drawMap(char map[MAXY][MAXX]);
void drawAll(gameState* game);
void drawInfo(gameState* game);
void drawGameOverScreen();
void drawWinnerScreen();

#endif