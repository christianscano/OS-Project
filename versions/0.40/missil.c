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

/* --- IMPLEMENTAZIONE FUNZIONI --- */

//Funzione routine thread missile 
void* missilRoutine(void* param) {
    argsThread* args = (argsThread*) param;

    while(!args->missil->data.isTerminated) {
        pthread_mutex_lock(&rw_mutex);
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
        pthread_mutex_unlock(&rw_mutex);
        usleep(1000);
    } 
    
    //Dealloco dati missile
    pthread_mutex_lock(&rw_mutex);
    deleteMissil(args->missilArray, args->missil);
    free(args);
    pthread_mutex_unlock(&rw_mutex);
}

//Inizializza la lista di missili
void inizialiseArrayMissil(listMissil* missilArray) {
    missilArray->dimArray = 0;
    missilArray->top = NULL;
}

//Reinizializza la lista di missili
void reinizialiseArrayMissil(listMissil* missilArray, char map[MAXY][MAXX]) {
    missil* it = missilArray->top;
    missil* aux;

    while(it != NULL) {
        aux = it->next;
        pthread_mutex_lock(&rw_mutex);
        if(getSquareMap(map, it->data.p_pos.y, it->data.p_pos.x) != '~' && getSquareMap(map, it->data.p_pos.y, it->data.p_pos.x) != '*')
            setSquareMap(map, it->data.p_pos.y, it->data.p_pos.x, ' ');
        deletePrevMissilPos(it->data, map);
        it->data.isTerminated = true;
        pthread_mutex_unlock(&rw_mutex);
        pthread_join(it->data.missilID, NULL);
        it = aux;
    }
}

//Viene chiamata alla pressione dello SPACE o in maniera radom da un ghost, carica quattro missili, uno in ogni direzione, all'interno della lista
//param isPacman => Indica se lo sparo è stato generato da pacman oppure no, in quest ultimo caso su index si trova l id del ghost che ha generato
//                  lo sparo
void loadMissil(gameState* game, bool isPacman, int index) {
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
        //Caricamento dati missile
        if(isPacman) {
            data.p_pos = game->pacman.p_pos;
            data.isPacman = true;
        } else {
            data.p_pos = game->ghostArray[index].p_pos;
            data.isPacman = false;
        }

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
            switch (getSquareMap(game->map, it->data.p_pos.y, it->data.p_pos.x)) {
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

    if(getSquareMap(map, data.p_pos.y, data.p_pos.x) != '~' && getSquareMap(map, data.p_pos.y, data.p_pos.x) != '*')
        setSquareMap(map, data.p_pos.y, data.p_pos.x, ' ');
}

//Funzione che gestisce lo scontro tra missili, pacman e ghosts in tutte le casistiche
void missilVsGhostVsPacman(ghost_object* ghostArray, pacman_object* pacman, listMissil* list, char map[MAXY][MAXX]) {
    int i;
    bool hit;
    missil* it = list->top; 

    while(it != NULL) {
        //Missile Vs Pacman
        if(!it->data.isTerminated && !it->data.isPacman && !pacman->isDead) { 
            if((pacman->p_pos.x == it->data.p_pos.x && pacman->p_pos.y == it->data.p_pos.y) ||
                (pacman->p_pos.x == it->data.p_pos.x && pacman->p_pos.y - 1 == it->data.p_pos.y) ||
                (pacman->p_pos.x == it->data.p_pos.x && pacman->p_pos.y + 1 == it->data.p_pos.y) ||
                (pacman->p_pos.x - 2 == it->data.p_pos.x && pacman->p_pos.y == it->data.p_pos.y) ||
                (pacman->p_pos.x + 2 == it->data.p_pos.x && pacman->p_pos.y == it->data.p_pos.y)) {
                
                deletePrevMissilPos(it->data, map);
                if(getSquareMap(map, it->data.p_pos.y, it->data.p_pos.x) != '~' && getSquareMap(map, it->data.p_pos.y, it->data.p_pos.x) != '*')
                    setSquareMap(map, it->data.p_pos.y, it->data.p_pos.x, ' ');
                it->data.isTerminated = true;
                if(++pacman->countMissil == 10)
                    pacman->isDead = true;
            }
        }
        
        //Missile Vs Ghosts
        if(!it->data.isTerminated)  
            for(i = 0; i < NUM_GHOST; i++) {
                if(!ghostArray[i].isHome && !ghostArray[i].insideHome) {
                    if(ghostArray[i].p_pos.x == it->data.p_pos.x && ghostArray[i].p_pos.y == it->data.p_pos.y)
                        hit = true;
                    else {
                        switch(it->data.missilDir){
                            case SU:
                                if(ghostArray[i].p_pos.y == it->data.p_pos.y - 1 && ghostArray[i].p_pos.x == it->data.p_pos.x)
                                    hit = true;
                                break;
                            case GIU:
                                if(ghostArray[i].p_pos.y == it->data.p_pos.y + 1 && ghostArray[i].p_pos.x == it->data.p_pos.x) 
                                    hit = true;
                                break;
                            case SINISTRA:
                                if(ghostArray[i].p_pos.y == it->data.p_pos.y && ghostArray[i].p_pos.x == it->data.p_pos.x - 2) 
                                    hit = true;
                                break;
                            case DESTRA:  
                                if(ghostArray[i].p_pos.y == it->data.p_pos.y && ghostArray[i].p_pos.x == it->data.p_pos.x + 2) 
                                    hit = true;
                                break;
                        }
                    }
                    //Se c'è stata una collisione
                    if(hit) {
                        it->data.isTerminated = true;
                        deletePrevMissilPos(it->data, map);
                        
                        if(getSquareMap(map, it->data.p_pos.y, it->data.p_pos.x) != '~' && getSquareMap(map, it->data.p_pos.y, it->data.p_pos.x) != '*')
                            setSquareMap(map, it->data.p_pos.y, it->data.p_pos.x, ' ');
                        
                        if(it->data.isPacman) {
                            if(++ghostArray[i].countMissil == 10)
                                ghostArray[i].isDead = true; 
                        }  
                        break;
                    }
                }
            }
        it = it->next;
    }
}


/* --- FUNZIONI GESTIONE LISTA --- */

//Funzione che alloca un nuovo nodo
missil* acquireMissil(missil_data data) {
    missil* new = (missil*) malloc(sizeof(missil));
    new->data = data;
    new->next = NULL;

    return new;
}

//Funzione che inserisce un nodo nella lista
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

//Funzione che elimina un nodo della lista
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