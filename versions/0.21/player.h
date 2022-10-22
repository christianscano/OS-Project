#ifndef _PLAYER_H
#define _PLAYER_H

/* --- DICHIARAZIONE FUNZIONI GLOBALI --- */

void* playerRoutine(void* param);
void updatePlayer(gameState* game);
void inizialisePlayer(pacman_object* pacman);

#endif