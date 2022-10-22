#ifndef _GAME_H
#define _GAME_H

#define MAXX 28
#define MAXY 31
#define MAXPILL 244
#define SU 65 					/* Freccia su */
#define GIU 66 					/* Freccia giu */
#define SINISTRA 68				/* Freccia sinsitra */
#define DESTRA 67				/* Freccia destra */

typedef enum { EASY, MEDIUM, HARD } difficulty;

typedef struct {
    int x, y;
} pos;

typedef struct {
    pos p_pos;
    int lives;
    int lastDirValid;
    int dirValid;
    int iFrame;
    int score;
    int nextGhostScore;
} pacman_object;

typedef struct {
        pacman_object pacman;
        int pill;
        difficulty mode;
} gameState;

void mainGame();

#endif