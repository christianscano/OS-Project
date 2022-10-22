#include <curses.h>
#include <pthread.h>
#include <unistd.h>
#include "game.h"
#include "area.h"
#include <mm_malloc.h>

/* --- DICHIARAZIONE DATI INTERNI --- */
typedef struct {
    listMissil* missilArray;
    missil* missil;
} argsThread;


/* --- DICHIARAZIONE FUNZIONI INTERNE --- */
void deletePrevMissilPos(missil_data data, char map[MAXY][MAXX]);
missil* acquireMissil(missil_data data);
missil* insertMissil(listMissil* list, missil_data);
void deleteMissil(listMissil* list, missil* missil);
void test(missil_data* data, pacman_object pacman);

/* --- IMPLEMENTAZIONE FUNZIONI --- */

//Funzione routine thread missile 
void* missilRoutine(void* param) {
    argsThread* args = (argsThread*) param;

    do {
        if(args->missil->data.isWrite) {
            switch(args->missil->data.missilDir) {
                case SU:
                    args->missil->data.p_pos.y -= 1;
                    break;
                case GIU:
                    args->missil->data.p_pos.y += 1;
                    break;
                case SINISTRA:
                    args->missil->data.p_pos.x -= 2;
                    break;
                case DESTRA:
                    args->missil->data.p_pos.x += 2;
                    break;
            }
            args->missil->data.isWrite = false;
            args->missil->data.isRead = true;
        }
    } while(!args->missil->data.isTerminated);
    
    //Dealloco dati missile
    pthread_mutex_lock(&rw_mutex);
    deleteMissil(args->missilArray, args->missil);
    free(args);
    pthread_mutex_unlock(&rw_mutex);

    pthread_exit(NULL);
}

//Inizializza la lista di missili
void inizialiseArrayMissil(gameState* game) {
    game->missilArray.dimArray = 0;
    game->missilArray.top = NULL;
}

//Viene chiamata alla pressione dello SPACE, carica quattro missili, uno in ogni direzione, all'interno della lista
void loadMissil(gameState* game) {
    int i;
    missil_data data;
    missil* aux = NULL;
    argsThread *args = NULL;

    for(i = 0; i < 4; i++) {
        switch(i) {
            case 0:
                data.missilDir = SU;
                break;
            case 1:
                data.missilDir = GIU;
                break;
            case 2:
                data.missilDir = SINISTRA;
                break;
            case 3:
                data.missilDir = DESTRA;
                break;
        }
        data.p_pos = game->pacman.p_pos;
        data.isTerminated = false;
        data.isRead = false;
        data.isWrite = true;

        //Viene inserito un nuovo missile all'interno della lista
        aux = insertMissil(&game->missilArray, data);
        
        //Caricamento parametri thread
        args = (argsThread*) malloc(sizeof(argsThread));
        args->missil = aux;
        args->missilArray = &game->missilArray;
        
        //Creazione thread
        pthread_create(&aux->data.missilID, NULL, &missilRoutine, args);
    }
}

//Aggiorna graficamente ogni missile attivo e controlla eventuali collisioni di quest'ultimi
void updateMissils(gameState* game) {
    missil* it = game->missilArray.top;
    while(it != NULL) {
        if(!it->data.isTerminated && it->data.isRead) {
            it->data.c = getSquareMap(game->map, it->data.p_pos.y, it->data.p_pos.x);

            switch (it->data.c) {
                case ' ':
                    deletePrevMissilPos(it->data, game->map);
                    setSquareMap(game->map, it->data.p_pos.y, it->data.p_pos.x, '.');
                    it->data.isRead = false;
                    it->data.isWrite = true;
                    break;
                default: //Oggetto non valido muro o vari
                    deletePrevMissilPos(it->data, game->map);
                    it->data.isTerminated = true;
                    break;
            }
        }
        it = it->next;
    }
}

//Cancella nello schermo la posizione precedente di un missile all'interno della matrice 
void deletePrevMissilPos(missil_data data, char map[MAXY][MAXX]) {
    switch (data.missilDir) {
        case SU:
            data.p_pos.y += 1;
            break;
        case GIU:
            data.p_pos.y -=1;
            break;
        case SINISTRA:
            data.p_pos.x += 2;
            break;
        case DESTRA:
            data.p_pos.x -= 2;
            break;
    }
    setSquareMap(map, data.p_pos.y, data.p_pos.x, ' ');
}

/* --- FUNZIONI GESTIONE LISTA --- */

missil* acquireMissil(missil_data data) {
    missil* new = (missil*) malloc(sizeof(missil));
    new->data = data;
    new->next = NULL;

    return new;
}

missil* insertMissil(listMissil* list, missil_data data) {
    missil* it = NULL, *new = NULL;
    
    new = acquireMissil(data);

    if(list->top == NULL) //Inserimento in testa 
        list->top = new;
    else {                //Inserimento in coda
        it = list->top;
        while(it->next != NULL)
            it = it->next;
        
        it->next = new;
    }

    list->dimArray++;
    return new;
}

void deleteMissil(listMissil* list, missil* m) {
    missil* it = list->top;
        
    if(list->top == m)  //Eliminazione in testa
        list->top = m->next;
    else {              //Eliminazione in coda o in mezzo
        while(it->next != NULL && it->next != m) 
            it = it->next;

        it->next = m->next;
    }
    
    list->dimArray--;
    free(m);
}