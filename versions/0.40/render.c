#include <curses.h>
#include <unistd.h>
#include <string.h>
#include "render.h"
#include "player.h"
#include "area.h"
#include "game.h"

/* --- MACRO --- */
#define WALL_COLOR 1
#define PACMAN_COLOR 2
#define PILL_COLOR 3
#define POWERPILL_COLOR 4
#define MISSIL_COLOR 5
#define GHOST_COLOR 20
#define GHOST_VULNERABLE 6

/* --- IMPLEMTENTAZIONE FUNZIONI --- */

//Funzione che configura all'inizio la libreria ncurses
void configCurses() {
    int i;
    int ghostColor[4] = {COLOR_RED, COLOR_GREEN, COLOR_MAGENTA, COLOR_CYAN };
    
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
    init_pair(GHOST_VULNERABLE, COLOR_BLUE, COLOR_WHITE);

    //Calibrazione di alcuni colori
    if (can_change_color()) {
		  init_color(COLOR_GREEN, 1000,644,888); //rosa
		  init_color(COLOR_MAGENTA, 498,1000,831); //verdeacqua
		  init_color(COLOR_CYAN, 1000,644,285); //Arancione
    }

    for(i = 0; i < NUM_GHOST; i++)
        init_pair(GHOST_COLOR+i, ghostColor[i % 4], COLOR_BLACK);
}

//Funzione che si occupa di disegnare l'intera mappa e gli oggetti al suo interno
void drawMap(char map[MAXY][MAXX]) {
    int i, j;
    char c;

    for(i = 0; i < MAXY; i++) {
        for(j = 0; j < MAXX; j++) {
            c = getSquareMap(map, i, j);
            switch (c) {
                case '-':
                    attron(A_BOLD);
                    attron(COLOR_PAIR(PACMAN_COLOR));
                    mvaddch(i, j, c);
                    attroff(COLOR_PAIR(PACMAN_COLOR));
                    attroff(A_BOLD);
                    break;
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
                    attron(A_BOLD);
                    attron(A_BLINK);
                    attron(COLOR_PAIR(POWERPILL_COLOR));
                    mvaddch(i, j, c);
                    attroff(COLOR_PAIR(POWERPILL_COLOR));
                    attroff(A_BLINK);
                    attroff(A_BOLD);
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
            }
        }  
        mvaddch(i, j, '\n');
    }
}

//Funzione che si occupa di disegnare pacman nello schermo
void drawPlayer(pacman_object pacman) {
    char frame[4][6] = {
    /*Sinistra*/	{ "})>->)" },
    /*Destra*/	    { "{(<-<(" },
    /*Sopra*/		{ "VVVV||" },
    /*Sotto*/	    { "^^^^||" },
    };

    int c;

    switch(pacman.lastDirValid) {
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

    attron(A_BOLD);
    attron(COLOR_PAIR(PACMAN_COLOR));
    mvaddch(pacman.p_pos.y, pacman.p_pos.x, frame[c][pacman.iFrame]);
    attroff(COLOR_PAIR(PACMAN_COLOR));
    attroff(A_BOLD);    
}

//Funzione che si occupa di disegnare l'animazione di morte di pacman
void drawPlayerDead(pacman_object pacman) {
    int iFrame = 0;
    char frame[] = "|Vv_.+*X*+. ";

    do {
        attron(A_BOLD | COLOR_PAIR(PACMAN_COLOR));
        mvaddch(pacman.p_pos.y, pacman.p_pos.x, frame[iFrame]);
        attroff(A_BOLD | COLOR_PAIR(PACMAN_COLOR));
        refresh();
        usleep(150000);
    } while(++iFrame < strlen(frame));
}

//Funzione che si occupa di disegnare i fantasmi nello schermo
void drawGhosts(ghost_object array[]) {
    int i, color;

    for(i = 0; i < NUM_GHOST; i++) {
        color = COLOR_PAIR(GHOST_COLOR + i);

        if(array[i].isVunerable) 
            color = COLOR_PAIR(GHOST_VULNERABLE);
        
        attron(A_BOLD);
        attron(color);
        mvaddch(array[i].p_pos.y, array[i].p_pos.x, 'M');
        attroff(color);
        attroff(A_BOLD);
    }
}

//Funzione che disegna tutti gli elementi a schermo
void drawAll(gameState* game) {
    drawMap(game->map);
    drawPlayer(game->pacman);
    drawGhosts(game->ghostArray);
    drawInfo(game);
    refresh();
}

//Funzione che disegna le info laterali
void drawInfo(gameState* game) {
    int cifra, n, x;
    
    mvprintw(0, 59, "___ _________  ______  _   _________  _____ ");
    mvprintw(1, 59, "|__]|__||   |\\/||__||\\ |   | __|__||\\/||___ ");
    mvprintw(2, 59, "|   |  ||___|  ||  || \\|   |__]|  ||  ||___ ");

    mvprintw(5, 59, "____________________  ");
    mvprintw(6, 59, "[__ |   |  ||__/|___. ");
    mvprintw(7, 59, "___]|___|__||  \\|___. ");

    n = game->pacman.score;
    x = 110;
    if(n != 0)
        while(n != 0) {
            cifra = n % 10;
            n = n / 10;
            switch (cifra) {
                case 0: 
                    mvprintw(4, x, "  __  ");
                    mvprintw(5, x, " /  \\ ");
                    mvprintw(6, x, "| () |");
                    mvprintw(7, x, " \\__/ ");
                    x -= 7;
                    break;
                case 1:
                    mvprintw(4, x, " _ ");
                    mvprintw(5, x, "/ |");
                    mvprintw(6, x, "| |");
                    mvprintw(7, x, "|_|");
                    x -= 7;
                    break;
                case 2:
                    mvprintw(4, x, " ___ ");
                    mvprintw(5, x, "|_  )");
                    mvprintw(6, x, " / / ");
                    mvprintw(7, x, "/___|");
                    x -= 6;
                    break;
                case 3:
                    mvprintw(4, x, " ____");
                    mvprintw(5, x, "|__ /");
                    mvprintw(6, x, " |_ \\");
                    mvprintw(7, x, "|___/");
                    x -= 6;
                    break;
                case 4:
                    mvprintw(4, x, " _ _  ");
                    mvprintw(5, x, "| | | ");
                    mvprintw(6, x, "|_  _|");
                    mvprintw(7, x, "  |_| ");
                    x -= 7;
                    break;
                case 5:
                    mvprintw(4, x, " ___ ");
                    mvprintw(5, x, "| __|");
                    mvprintw(6, x, "|__ \\");
                    mvprintw(7, x, "|___/");
                    x -= 6;
                    break;
                case 6:
                    mvprintw(4, x, "  __ ");
                    mvprintw(5, x, " / / ");
                    mvprintw(6, x, "/ _ \\");
                    mvprintw(7, x, "\\___/");
                    x -= 6;
                    break;
                case 7:
                    mvprintw(4, x, " ____ ");
                    mvprintw(5, x, "|__  |");
                    mvprintw(6, x, "  / / ");
                    mvprintw(7, x, " /_/  ");
                    x -= 7;
                    break;
                case 8:
                    mvprintw(4, x, " ___ ");
                    mvprintw(5, x, "( _ )");
                    mvprintw(6, x, "/ _ \\");
                    mvprintw(7, x, "\\___/");
                    x -= 6;
                    break;
                case 9:
                    mvprintw(4, x, " ___ ");
                    mvprintw(5, x, "/ _ \\");
                    mvprintw(6, x, "\\_, /");
                    mvprintw(7, x, " /_/ ");
                    x -= 6;
                    break;

            }
        }
    
    mvprintw(9, 59, "_   __  _________  ");
    mvprintw(10, 59, "|   ||  ||___[__ . ");
    mvprintw(11, 59, "|___| \\/ |______]. ");
    switch (game->pacman.lives) {
        case 0: 
            mvprintw(8, 81, "  __  ");
            mvprintw(9, 81, " /  \\ ");
            mvprintw(10, 81, "| () |");
            mvprintw(11, 81, " \\__/ ");
            break;
        case 1:
            mvprintw(8, 81, " _ ");
            mvprintw(9, 81, "/ |");
            mvprintw(10, 81, "| |");
            mvprintw(11, 81, "|_|");
            break;
        case 2:
            mvprintw(8, 81, " ___ ");
            mvprintw(9, 81, "|_  )");
            mvprintw(10, 81, " / / ");
            mvprintw(11, 81, "/___|");
            break;
        case 3:
            mvprintw(8, 81, " ____");
            mvprintw(9, 81, "|__ /");
            mvprintw(10, 81, " |_ \\");
            mvprintw(11, 81, "|___/");
            break;
        case 4:
            mvprintw(8, 81, " _ _  ");
            mvprintw(9, 81, "| | | ");
            mvprintw(10, 81, "|_  _|");
            mvprintw(11, 81, "  |_| ");
            break;
        case 5:
            mvprintw(8, 81, " ___ ");
            mvprintw(9, 81, "| __|");
            mvprintw(10, 81, "|__ \\");
            mvprintw(11, 81, "|___/");
            break;
        case 6:
            mvprintw(8, 81, "  __ ");
            mvprintw(9, 81, " / / ");
            mvprintw(10, 81, "/ _ \\");
            mvprintw(11, 81, "\\___/");
            break;
    }   
}

//Funzione che disegna la schermata di game over
void drawGameOverScreen() {
    erase();
    mvprintw(5, 1, "____ ____ _  _ ____    ____ _  _ ____ ____ ");
    mvprintw(6, 1, "| __ |__| |\\/| |___    |  | |  | |___ |__/ ");
    mvprintw(7, 1, "|__] |  | |  | |___    |__|  \\/  |___ |  \\ ");
    mvprintw(8, 1, "Premere invio due volte per uscire.");
    refresh();
    getch();
}

//Funzione che disegna la schermata di vittoria
void drawWinnerScreen() {
    erase();
    mvprintw(5, 1, "_ _ _ _ _  _ _  _ ____ ____ ");
    mvprintw(6, 1, "| | | | |\\ | |\\ | |___ |__/ ");
    mvprintw(7, 1, "|_|_| | | \\| | \\| |___ |  \\ ");
    mvprintw(8, 1, "Premere invio due volte per uscire.");
    refresh();
    getch();
}