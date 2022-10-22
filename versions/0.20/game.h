#ifndef _GAME_H
#define _GAME_H

#include <pthread.h>
#include <stdbool.h>

/* --- MACRO --- */
#define MAXX 55                 //Larghezza massima campo
#define MAXY 31                 //Lunghezza massima campo 
#define MAXPILL 244             //Numero totale di pillole
#define SU 65 					//Freccia su 
#define GIU 66 					//Freccia giu 
#define SINISTRA 68				//Freccia sinsitra
#define DESTRA 67				//Freccia destra
#define SPACE 32                //Tasto space 


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

pthread_mutex_t mtx;
//pthread_mutex_t mtxAreaGioco;
//pthread_mutex_t mtxListMissil;

typedef struct {
    pos p_pos;              //Posizione pacman
    int lives;              //Numero vite
    int lastDirValid;       //Ultima direzione inserita valida
    int dirValid;           //Ultima direzione inserita
    int iFrame;             //Frame attuale animazione
    int score;              //Contattore punteggio
    int nextGhostScore;     //Moltiplicatore fantasma 
    bool isSpare;           //Verifica se si è generato uno sparo
} pacman_object;            //Struttura oggetto pacman

typedef struct {
    pacman_object pacman;   //Oggetto pacman

    listMissil missilArray; //Lista Missili
        
    int pill;               //Numero pillole rimanenti
        
    difficulty mode;        //Difficolta gioco scelta
} gameState;                //Struttura dati game


/* --- DICHIARAZIONE FUNZIONI GLOBALI --- */
void mainGame();

#endif