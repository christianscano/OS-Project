#include <curses.h>
#include "render.h"
#include "player.h"
#include "area.h"
#include "game.h"

#define WALL_COLOR 1
#define PACMAN_COLOR 2
#define PILL_COLOR 3
#define POWERPILL_COLOR 4
#define MISSIL_COLOR 5


//GHOST COLOR red, green, cyan, magenta

/* --- IMPLEMTENTAZIONE FUNZIONI --- */

//Funzione che configura all'inizio la libreria ncurses
void configCurses() {
    initscr();
    noecho();
    curs_set(FALSE);
    start_color();

    //Inizializzazione colori
    init_pair(WALL_COLOR, COLOR_BLUE, COLOR_BLACK);
    init_pair(PILL_COLOR, COLOR_CYAN, COLOR_BLACK);
    init_pair(POWERPILL_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(PACMAN_COLOR, COLOR_YELLOW, COLOR_BLACK);
    init_pair(MISSIL_COLOR, COLOR_RED, COLOR_BLACK);

    //Calibrazione di alcuni colori
    if (can_change_color()) {
		  init_color(COLOR_GREEN, 1000,644,888);
		  init_color(COLOR_MAGENTA, 1000,888,970);
		  init_color(COLOR_CYAN, 1000,644,285); //Arancione
    }
}

//Funzione che si occupa di disegnare l'intera mappa e gli oggetti al suo interno
void drawMap() {
    int i, j;
    char c;

    for(i = 0; i < MAXY; i++) {
        for(j = 0; j < MAXX; j++) {
        c = getSquareMap(i, j);
        switch (c) {
            case '-':
                attron
            case '0':
                attron(A_BOLD);
                attron(COLOR_PAIR(WALL_COLOR));
                mvaddch(i, j, NCURSES_ACS(c));
                attroff(COLOR_PAIR(WALL_COLOR));
                attroff(A_BOLD);
                break;
            case '~': 
                attron(COLOR_PAIR(PILL_COLOR));
                mvaddch(i, j, NCURSES_ACS(c));
                attroff(COLOR_PAIR(PILL_COLOR));
                break;
            case '*':
                attron(COLOR_PAIR(POWERPILL_COLOR));
                mvaddch(i, j, c);
                attroff(COLOR_PAIR(POWERPILL_COLOR));
                break;
            case ' ':
                mvaddch(i, j, ' ');
                break;
            case '.':
                attron(A_BOLD);
                attron(COLOR_PAIR(MISSIL_COLOR));
                mvaddch(i, j, c);
                attroff(COLOR_PAIR(MISSIL_COLOR));
                attroff(A_BOLD);
            break;
            default: //pacman
            attron(A_BOLD);
            attron(COLOR_PAIR(PACMAN_COLOR));
            mvaddch(i, j, c);
            attroff(COLOR_PAIR(PACMAN_COLOR));
            attroff(A_BOLD);
            }
        }  
        mvaddch(i, j, '\n');
    }
}

//Funzione che si occupa di disegnare pacman all'interno della mappa
void drawPlayer(pacman_object* pacman) {
    char frame[4][6] = {
    /*Sinistra*/	{ "})>->)" },
    /*Destra*/	{ "{(<-<(" },
    /*Sopra*/		{ "VVVV||" },
    /*Sotto*/	{ "^^^^||" },
    };

    int c;

    switch(pacman->lastDirValid) {
        case SU: 
        c = 2;
        break;
        case GIU:
        c = 3;
        break;
        case SINISTRA: 
        c = 0;
        break;
        case DESTRA:
        c = 1;
        break;
    }

    setSquareMap(pacman->p_pos.y, pacman->p_pos.x, frame[c][pacman->iFrame]);
}
