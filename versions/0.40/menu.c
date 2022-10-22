#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

/* --- MACRO --- */
#define DIM_MAIN_VOICE 9
#define DIM_LEVEL_VOICE 12
#define DIM_STRING 50

/* --- DICHIARAZIONE FUNZIONI INTERNE --- */
void printTitle(int x);
void loadVoiceMenu(WINDOW* w, char m[][DIM_STRING], int dim, int maxx);
int mainMenu(WINDOW *w, char mainVoice[DIM_MAIN_VOICE][DIM_STRING], int maxx);
int subMenuMode(WINDOW* w, char level[DIM_LEVEL_VOICE][DIM_STRING], int maxx);
void printInstruction(WINDOW *w);

/* --- DATI INTERNI --- */

char mainVoice[DIM_MAIN_VOICE][DIM_STRING] = { 
        "___  _    ____ _   _ ", "|__] |    |__|  \\_/  ", "|    |___ |  |   |   ",
        "_ ____ ___ ____ _  _ ____ ___ _ ____ _  _ ____ ", "| [__   |  |__/ |  | |     |  | |  | |\\ | [__  ", "| ___]  |  |  \\ |__| |___  |  | |__| | \\| ___] ",
        "____ _  _ _ ___ ", "|___  \\/  |  |  ", "|___ _/\\_ |  |  "
};

char level[DIM_LEVEL_VOICE][DIM_STRING] = {
        "____ ____ ____ _   _ ", "|___ |__| [__   \\_/  ", "|___ |  | ___]   |   ",
        "_  _ ____ ____ _  _ ____ _    ", "|\\ | |  | |__/ |\\/| |__| |    ", "| \\| |__| |  \\ |  | |  | |___ ",
        "_  _ ____ ____ ___  ", "|__| |__| |__/ |  \\ ", "|  | |  | |  \\ |__/ ",
        "____ ____ ___ _  _ ____ _  _ ", "|__/ |___  |  |  | |__/ |\\ | ", "|  \\ |___  |  |__| |  \\ | \\| "
};

/* --- IMPLEMENTAZIONE FUNZIONI --- */

//Funzione principale del menu
int loadMenu(gameState* game) {
    int maxx;

    maxx = getmaxx(stdscr);
    printTitle(maxx / 2 - 38);

	WINDOW *w = newwin( 0, 0, 10, 0); // create a new window
    wrefresh(w);

    while(true) {
        switch(mainMenu(w, mainVoice, maxx)) {
            case 0: //Launch sub menu mode
                switch(subMenuMode(w, level, maxx)) {
                    case 0:
                    case 1:
                    case 2: game->mode = EASY; return 0; break;
                    case 3:
                    case 4:
                    case 5: game->mode = NORMAL; return 0; break;
                    case 6:
                    case 7:
                    case 8: game->mode = HARD; return 0; break;
                }
                break;
            case 3: //Launch instruction
                printInstruction(w);
                break;
            case 6: return -1; //Exit
        }
    }
    delwin(w); 
}

//Funzione che stampa il titolo
void printTitle(int x) {
    init_pair(10, COLOR_YELLOW, COLOR_BLACK);
    attron(COLOR_PAIR(10));
    mvprintw(0, x, "   _ (`-.     ('-.                  _   .-')       ('-.          .-') _  ");
    mvprintw(1, x, "  ( (OO  )   ( OO ).-.             ( '.( OO )_    ( OO ).-.     ( OO ) ) ");
    mvprintw(2, x, " _.`     \\   / . --. /    .-----.   ,--.   ,--.)  / . --. / ,--./ ,--,'  ");
    mvprintw(3, x, "(__...--''   | \\-.  \\    '  .--./   |   `.'   |   | \\-.  \\  |   \\ |  |\\  ");
    mvprintw(4, x, " |  /  | | .-'-'  |  |   |  |('-.   |         | .-'-'  |  | |    \\|  | ) ");
    mvprintw(5, x, " |  |_.' |  \\| |_.'  |  /_) |OO  )  |  |'.'|  |  \\| |_.'  | |  .     |/ ");
    mvprintw(6, x, " |  .___.'   |  .-.  |  ||  |`-'|   |  |   |  |   |  .-.  | |  |\\    |   ");
    mvprintw(7, x, " |  |        |  | |  | (_'  '--'\\   |  |   |  |   |  | |  | |  | \\   |   ");
    mvprintw(8, x, " `--'        `--' `--'    `-----'   `--'   `--'   `--' `--' `--'  `--'   ");
    attroff(COLOR_PAIR(10));
    refresh();
}

//Funzione che stampa le istruzioni
void printInstruction(WINDOW *w) {
    wclear(w);
    wrefresh(w);
    wprintw(w, "\tQuesto gioco è una versione alterniva del gioco originale pacman, poichè sia i ghost sia pacman possono generare spari.\n");
    wprintw(w, "\tSi hanno a disposizione 3 vite e il vostro compito sara quello di raccogliere tutti i pallini nella mappa. State attenti poichè\n"); 
    wprintw(w, "\togni 10 missili ricevuti si perdera una vita, questa regola vale anche per i ghost.\n");
    wprintw(w, "\tComandi pacman : il movimento avviene attraverso le freccette direzionali mentre lo sparo con la pressione della barra spaziatrice.\n");
    wprintw(w, "\tQuesta versione dispone di 3 livelli di difficoltà: EASY, NORMAL, HARD.\n");
    wprintw(w, "\tIn modalita EASY i ghosts si muoverano in maniera randomica e la loro velocità è inferiore alla vostra.\n");
    wprintw(w, "\tIn modalita NORMAL i ghosts una volta raggiunta una certa distanza incomincerano a seguirvi, la loro velocità è inferiore alla vostra.\n");
    wprintw(w, "\tIn modalita HARD i ghost una volta raggiunta una certa distanza incomincerano a seguirvi, la loro velocità sara uguale alla vostra.\n");
    wprintw(w, "\n\tBuon Game.\n");
    wprintw(w, "\tPremere un tasto per tornare indietro...");
    wrefresh(w);
    wgetch(w);
    
}

//Funzione che stampa le voci del menu
void loadVoiceMenu(WINDOW* w, char m[][50], int dim, int maxx) {
    int i;
    wclear(w);
    wrefresh(w);

    for(i = 0; i < dim; i+=3) {
        if(i == 0) {
            wattron(w, A_BOLD);
            wattron(w, A_BLINK);
        }
        else {
            wattroff(w, A_BOLD);
            wattroff(w, A_BLINK);
        }
        mvwprintw(w, i+1, 2, "%*s", (maxx / 2 - 3 + strlen(m[i]) / 2), m[i]);
        mvwprintw(w, i+2, 2, "%*s", (maxx / 2 - 3 + strlen(m[i+1]) / 2), m[i+1]);
        mvwprintw(w, i+3, 2, "%*s", (maxx / 2 - 3 + strlen(m[i+2]) / 2), m[i+2]);
    }

    wrefresh(w);
}

//Funzione che visualizza il main menu
int mainMenu(WINDOW *w, char mainVoice[9][50], int maxx) {
    int i = 0;
    char c;

    loadVoiceMenu(w, mainVoice, 9, maxx);

    while(true) { 
        c = wgetch(w);
        
        mvwprintw(w, i+1, 2, "%*s", (maxx / 2 - 3 + strlen(mainVoice[i]) / 2), mainVoice[i]);
        mvwprintw(w, i+2, 2, "%*s", (maxx / 2 - 3 + strlen(mainVoice[i+1]) / 2), mainVoice[i+1]);
        mvwprintw(w, i+3, 2, "%*s", (maxx / 2 - 3 + strlen(mainVoice[i+2]) / 2), mainVoice[i+2]);
        
        switch(c) {
            case SU:
                i -= 3;
                i = (i < 0) ? 6 : i;
                break;
            case GIU:
                i += 3;
                i = (i > 8) ? 0 : i;
                break;
            case '\n':
                return i;
        }
            
        wattron(w, A_BOLD);
        wattron(w, A_BLINK);
        mvwprintw(w, i+1, 2, "%*s", (maxx / 2 - 3 + strlen(mainVoice[i]) / 2), mainVoice[i]);
        mvwprintw(w, i+2, 2, "%*s", (maxx / 2 - 3 + strlen(mainVoice[i+1]) / 2), mainVoice[i+1]);
        mvwprintw(w, i+3, 2, "%*s", (maxx / 2 - 3 + strlen(mainVoice[i+2]) / 2), mainVoice[i+2]);
        wattroff(w, A_BLINK);
        wattroff(w, A_BOLD);
    }
}

//Funzione che visualizza il menu dei livelli di gioco
int subMenuMode(WINDOW* w, char level[12][50], int maxx) {
    int i;
    char c;

    loadVoiceMenu(w, level, 12, maxx);
    while(true) { 
        c = wgetch(w);
        
        mvwprintw(w, i+1, 2, "%*s", (maxx / 2 - 3 + strlen(level[i]) / 2), level[i]);
        mvwprintw(w, i+2, 2, "%*s", (maxx / 2 - 3 + strlen(level[i+1]) / 2), level[i+1]);
        mvwprintw(w, i+3, 2, "%*s", (maxx / 2 - 3 + strlen(level[i+2]) / 2), level[i+2]);
        
        switch(c) {
            case SU: 
                i -= 3;
                i = (i < 0) ? 9 : i;
                break;
            case GIU:
                i += 3;
                i = (i > 11) ? 0 : i;
                break;
            case '\n':
                return i;
        }
            
        wattron(w, A_BOLD);
        wattron(w, A_BLINK);
        mvwprintw(w, i+1, 2, "%*s", (maxx / 2 - 3  + strlen(level[i]) / 2), level[i]);
        mvwprintw(w, i+2, 2, "%*s", (maxx / 2 - 3 + strlen(level[i+1]) / 2), level[i+1]);
        mvwprintw(w, i+3, 2, "%*s", (maxx / 2 - 3 + strlen(level[i+2]) / 2), level[i+2]);
        wattroff(w, A_BLINK);
        wattroff(w, A_BOLD);
    }
}