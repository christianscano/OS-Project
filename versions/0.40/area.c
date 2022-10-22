#include <stdio.h>
#include <curses.h>
#include <stdbool.h>
#include <string.h>
#include "render.h"
#include "player.h"
#include "area.h"
#include "game.h"

/* --- DATI INTERNI --- */

//Matrice che rappresenta l'intera area di gioco default
char map [MAXY][MAXX] = {
    { "0000000000000000000000000000000000000000000000000000000"},
    { "0 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 000 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 0"},
    { "0 ~ 0000000 ~ 000000000 ~ 000 ~ 000000000 ~ 0000000 ~ 0"},
    { "0 * 0000000 ~ 000000000 ~ 000 ~ 000000000 ~ 0000000 * 0"},
    { "0 ~ 0000000 ~ 000000000 ~ 000 ~ 000000000 ~ 0000000 ~ 0"},
    { "0 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 0"},
    { "0 ~ 0000000 ~ 000 ~ 000000000000000 ~ 000 ~ 0000000 ~ 0"},
    { "0 ~ 0000000 ~ 000 ~ 000000000000000 ~ 000 ~ 0000000 ~ 0"},
    { "0 ~ ~ ~ ~ ~ ~ 000 ~ ~ ~ ~ 000 ~ ~ ~ ~ 000 ~ ~ ~ ~ ~ ~ 0"},
    { "00000000000 ~ 000000000   000   000000000 ~ 00000000000"},
    { "         00 ~ 000000000   000   000000000 ~ 00         "},
    { "         00 ~ 000                     000 ~ 00         "},
    { "         00 ~ 000   000000---000000   000 ~ 00         "},
    { "00000000000 ~ 000   00           00   000 ~ 00000000000"},
    { "            ~       00           00       ~            "},
    { "00000000000 ~ 000   00           00   000 ~ 00000000000"},
    { "         00 ~ 000   000000000000000   000 ~ 00         "},
    { "         00 ~ 000                     000 ~ 00         "},
    { "         00 ~ 000   000000000000000   000 ~ 00         "},
    { "00000000000 ~ 000   000000000000000   000 ~ 00000000000"},
    { "0 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 000 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 0"},
    { "0 ~ 0000000 ~ 000000000 ~ 000 ~ 000000000 ~ 0000000 ~ 0"},
    { "0 ~ 0000000 ~ 000000000 ~ 000 ~ 000000000 ~ 0000000 ~ 0"},
    { "0 * ~ ~ 000 ~ ~ ~ ~ ~ ~ ~     ~ ~ ~ ~ ~ ~ ~ 000 ~ ~ * 0"},
    { "00000 ~ 000 ~ 000 ~ 000000000000000 ~ 000 ~ 000 ~ 00000"},
    { "00000 ~ 000 ~ 000 ~ 000000000000000 ~ 000 ~ 000 ~ 00000"},
    { "0 ~ ~ ~ ~ ~ ~ 000 ~ ~ ~ ~ 000 ~ ~ ~ ~ 000 ~ ~ ~ ~ ~ ~ 0"},
    { "0 ~ 0000000000000000000 ~ 000 ~ 0000000000000000000 ~ 0"},
    { "0 ~ 0000000000000000000 ~ 000 ~ 0000000000000000000 ~ 0"},
    { "0 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 0"},
    { "0000000000000000000000000000000000000000000000000000000"}
};

/* --- IMPLEMENTAZIONE FUNZIONI --- */

//Restituisce un elemento della mappa
char getSquareMap(char map[MAXY][MAXX], int y, int x) {
    return map[y][x];
}

//Controlla se in una data posizione non vi è un muro, TRUE se è valida
bool isValidSquare(char map[MAXY][MAXX], int y, int x) {
    if(getSquareMap(map, y, x) != '0' && getSquareMap(map, y, x) != '-')
        return true;
    return false;
}

//Permette si modificare un quadratto della mappa se si inserisce un valore valido 
void setSquareMap(char map[MAXY][MAXX], int y, int x, char c) {
    if(isValidSquare(map, y, x))
        map[y][x] = c;
}

//Ripopola la mappa di gioco e reseta le variabili riguardanti la mappa
void repopulateMap(gameState* game) {
    int x, y;
    
    game->pill = 0;
    game->isEatPowerPill = false;

    memcpy(game->map, map, sizeof(map));

    for(y = 0; y < MAXY; y++)
        for(x = 0; x < MAXX; x++)
            if(getSquareMap(game->map, y, x) == '~' || getSquareMap(game->map, y, x) == '*')
                game->pill++;
}