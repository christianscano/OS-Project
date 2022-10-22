#ifndef _GAME_H
#define _GAME_H

#include <pthread.h>
#include <stdbool.h>

/* --- MACRO --- */
#define MAXX 55                 //Larghezza massima campo
#define MAXY 31                 //Lunghezza massima campo 
#define NUM_PILL 244            //Numero totale di pillole
#define MAXPACMANLIVES 6        //Numero massimo di vite di pacman
#define SU 65 					//Freccia su 
#define GIU 66 					//Freccia giu 
#define SINISTRA 68				//Freccia sinsitra
#define DESTRA 67				//Freccia destra
#define SPACE 32                //Tasto space 
#define NUM_GHOST 4             //Numero fantasmi in gioco (modificare nel caso si voglia aumentare il numerom di fantasmi)

/* ---STRUTTURE DATI --- */
typedef struct {
    int x, y;
} pos;                      //Struttura coordinate


/* --- STRUTTURE DATI LISTA MISSILE --- */
typedef struct {
    pos p_pos;              //Posizione missile
    int missilDir;          //Direzione missile 
    pthread_t missilID;     //Indentificatore thread missile
    bool isTerminated;      //Flag indica se il missile si è scontratto con qualcosa
    bool isRead;            //Flag indica se si possono leggere le nuove coordinate
    bool isWrite;           //Flag indica se si possono aggiornare le coordinate
    char c;                 //Valore letto nella posizione successiva
} missil_data;              //Struttura oggetto missile

typedef struct node {
    missil_data data;       //Dati missile
    struct node* next;      //Puntatore al nodo successivo
} missil;                   //Struttura singolo nodo lista 

typedef struct {
    int dimArray;           //Numero di nodi nella lista
    missil* top;            //Puntatore alla testa della lista
} listMissil;               //Struttura lista missili


/* --- DATI GIOCO --- */
typedef enum { EASY, MEDIUM, HARD } difficulty; //Livelli difficoltà 
typedef enum { PACMAN_DIE, COMPLETE_GAME, END_GAME } statusGame; 

pthread_mutex_t rw_mutex;   //Mutex

typedef struct {
    pos p_pos;              //Posizione pacman
    pthread_t pacmanID;     //Indicatore thread pacman
    int lives;              //Numero vite
    int lastDirValid;       //Ultima direzione inserita valida
    int dirValid;           //Ultima direzione inserita
    int iFrame;             //Frame attuale animazione
    int speed;              //Velocità movimento su schermo
    int score;              //Contattore punteggio
    int nextGhostScore;     //Moltiplicatore fantasma 
    bool isSpare;           //Flag indica se si è generato uno sparo
} pacman_object;            //Struttura oggetto pacman

typedef struct{
    pos p_pos;              //Posizione ghost
    pthread_t ghostID;      //Indicatore thread ghost
    int color;              //ID colore ghost
    int lives;              //Numero vite
    int lastDirValid;       //Ultima direzione valida
    int speed;              //Velocità movimento su schermo
    bool isHome;
    bool isVunerable;
    bool isSpare;           //Flag indica se si è generato uno sparo
    bool isRead;            //Flag indica se si possono leggere le nuove coordinate
    bool isWrite;           //Flag indica se si possono aggiornare le coordinate
} ghost_object;             //Struttura oggetto ghost


typedef struct {
    pacman_object pacman;   //Oggetto pacman

    ghost_object* ghostArray; 

    listMissil missilArray; //Lista Missili

    int pill;               //Numero pillole rimanenti
    char map[MAXY][MAXX];

    statusGame status;    
    difficulty mode;        //Difficolta gioco scelta
} gameState;                //Struttura dati game


/* --- DICHIARAZIONE FUNZIONI GLOBALI --- */
void mainGame();

#endif