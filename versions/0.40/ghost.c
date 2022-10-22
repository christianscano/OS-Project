#include <pthread.h>
#include <curses.h>
#include <unistd.h>
#include <mm_malloc.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "game.h"
#include "area.h"
#include "missil.h"

/* --- DATI INTERNI --- */
typedef struct {
    ghost_object* ghost;
    pos* posPacman;
    char (*map)[MAXY][MAXX]; //Puntatore a matrice
    difficulty mode;
} argsThread;

/* --- DICHIARAZIONE FUNZIONI INTERNE --- */
pos generaCoordinate(char map[MAXY][MAXX], ghost_object* array, int id);
void generaDirIniziale(ghost_object* ghost);
bool simulatePass(char map[MAXY][MAXX], pos* p_pos, int dir, bool isHome);
void* ghostRoutine(void* param);
float distanceToPacman(pos posGhost, pos posPacman);
void ghostAI(ghost_object* ghost, pos posPacman, char map[MAXY][MAXX]);
void ghostRandom(ghost_object* ghost, char map[MAXY][MAXX]);
void reboundEffect(int index, ghost_object* array, char map[MAXY][MAXX]);
void inverseDir(ghost_object* ghost, bool onlyDir, char map[MAXY][MAXX]);
void goToHome(ghost_object* ghost, char map[MAXY][MAXX]);
void startGhostThread(gameState* game, bool isHome, int index);
void goOutHome(ghost_object* ghost, char map[MAXY][MAXX]);
void reboundPass(ghost_object* ghost, char map[MAXY][MAXX]);

/* --- IMPLEMENTAZIONE FUNZIONI --- */

//Inizializza tutti i fantasmi
void inizialiseGhosts(gameState* game) {
    int i;
    
    game->ghostArray = (ghost_object*) malloc(sizeof(ghost_object) * NUM_GHOST);

    for(i = 0; i < NUM_GHOST; i++){
        game->ghostArray[i].p_pos = generaCoordinate(game->map, game->ghostArray, i);
        game->ghostArray[i].isDead = false; 
        generaDirIniziale(&game->ghostArray[i]);
        game->ghostArray[i].isWrite = true;
        game->ghostArray[i].isRead = false;
        game->ghostArray[i].isVunerable = false;
        game->ghostArray[i].isHome = false;
        game->ghostArray[i].insideHome = false;
        game->ghostArray[i].countMissil = 0;
        game->ghostArray[i].rebound = false;

        switch(game->mode) {
            case EASY:
            case NORMAL: game->ghostArray[i].speed = 2; break;
            case HARD: game->ghostArray[i].speed = 1; break;
        }
    }
}

//Effettua la chiusura i thread dei ghosts
void freeGhosts(gameState* game) {
    int i;
    for(i = 0; i < NUM_GHOST; i++) {
        pthread_mutex_lock(&rw_mutex);
        game->ghostArray[i].isDead = true;
        pthread_mutex_unlock(&rw_mutex);
        pthread_join(game->ghostArray[i].ghostID, NULL);
    }
}

//Rinizializza tutti i fantasmi
void reinizialiseGhosts(gameState* game) {
    freeGhosts(game);
    inizialiseGhosts(game);
}

//Lancia tutti thread per ogni ghost
void startGhostsThread(gameState* game) {
    int i;
    argsThread* args = NULL;

    for(i = 0; i < NUM_GHOST; i++) 
        startGhostThread(game, false, i);
}

//Lancia un singolo thread relativo a un ghost
//param isHome => Indica se il thread generato appartinene ad un ghost che deve ritornare nella home
void startGhostThread(gameState* game, bool isHome, int index) {
    argsThread* args = NULL;

    if(isHome)
        game->ghostArray[index].isHome = true;

    //Caricamento parametri thread
    args = (argsThread*) malloc(sizeof(argsThread));
    args->ghost = &game->ghostArray[index];
    args->posPacman = &game->pacman.p_pos;
    args->map = &game->map;
    args->mode = game->mode;

    //Creazione thread
    pthread_create(&game->ghostArray[index].ghostID, NULL, &ghostRoutine, args);
}

//Funzione thread ghost
void* ghostRoutine(void* param) {
    argsThread* args = (argsThread*) param;
    bool startCount = false, startCount2 = false;
    time_t t, t2;
    int sleep;

    //Se il ghost è appena stato generato attende un tempo random prima di iniziare a muoversi
    if(!args->ghost->isHome) {
        sleep = (1 + rand() % 3) * 1000000;
        usleep(sleep);  //Tempo di attesa random da 1 a 3 secondi prima che il ghost inizi a muoversi
    }

    while(!args->ghost->isDead) {
        pthread_mutex_lock(&rw_mutex);
        if(args->ghost->isWrite) {
            if(args->ghost->speed-- == 0) {
                if(!args->ghost->isHome && !args->ghost->insideHome) { //Se il ghost non è stato ucciso
                    switch(args->mode) {
                        case EASY:
                        case NORMAL: args->ghost->speed = 2; break;
                        case HARD: args->ghost->speed = 1; break;
                    } 

                    //Controllo se il fantasma è diventato vulnerabile inizio il conteggio
                    if(args->ghost->isVunerable && !startCount) {
                        t = time((time_t*)NULL);
                        startCount = true;
                    }

                    //Movimento ghost
                    switch(args->mode) {
                        case EASY:
                            ghostRandom(args->ghost, *args->map);
                            break;
                        case NORMAL:
                        case HARD:
                            if(args->ghost->isVunerable) 
                                ghostRandom(args->ghost, *args->map);
                            else if(args->ghost->rebound) { //Se si è verificato un rimbalzo
                                args->ghost->rebound = false; 
                                ghostRandom(args->ghost, *args->map);
                            }
                            else
                                ghostAI(args->ghost, *args->posPacman, *args->map);
                            break;
                    }

                    //Se sono passati 10 secondi da quando il fantasma è diventato vulnerabile, l'effetto della powerpill svanisce
                    if((time((time_t*)NULL) - t) > 10 && startCount) {
                        args->ghost->isVunerable = false;
                        startCount = false;
                    }
                } else {                    
                    args->ghost->speed = 0;
                    //Controllo se il ghost è entrato dentro la home
                    if(args->ghost->p_pos.x == 26 && args->ghost->p_pos.y == 13) {
                        args->ghost->insideHome = true;
                        args->ghost->isHome = false;
                    }
                    
                    //Se è appena entrato nella home faccio partire il timer  
                    if(!startCount2 && args->ghost->insideHome) {
                        t2 = time((time_t*)NULL);
                        startCount2 = true;
                    }

                    //Se sono passati 10 secondi da quando il ghost è dentro la home
                    if(startCount2 && (time((time_t*)NULL) - t2) > 10)
                        if(args->ghost->p_pos.x == 26 && args->ghost->p_pos.y == 11) {
                            args->ghost->insideHome = false;
                            startCount2 = false;
                            args->ghost->rebound = false;
                        } else
                            goOutHome(args->ghost, *args->map);
                    else if(startCount2) {  //Movimento all'interno della home
                        ghostRandom(args->ghost, *args->map);
                    } else
                        goToHome(args->ghost, *args->map); 
                }        
            }
            args->ghost->isWrite = false;
            args->ghost->isRead = true;
        }
        pthread_mutex_unlock(&rw_mutex);
        usleep(5000);
    }
    
    //Deallocamento parametri thread
    args->ghost->isRead = true;
    free(args); 
}

//Effettua in maniera intelligente, ovvero in base alla posizione di pacman, il prossimo passo del ghost
void ghostAI(ghost_object* ghost, pos posPacman, char map[MAXY][MAXX]) {
    int distanceDir[4] = { -1, -1, -1, -1 };
    int countDirValid = 0, min = -1, i;
    pos nextPos;

    //Controllo partendo dall'ultima direzione usata quali direzioni sono disponibili per il movimento
    switch(ghost->lastDirValid) {
        case SU:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SU, false)) { //Ultima direzione usata
                distanceDir[0] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA, false)) {
                distanceDir[3] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA, false)) {
                distanceDir[2] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            break;

        case GIU:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU, false)) { //Ultima direzione usata
                distanceDir[1] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA, false)) {
                distanceDir[3] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA, false)) {
                distanceDir[2] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            break;
        
        case SINISTRA:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA, false)) { //Ultima direzione usata
                distanceDir[2] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SU, false)) {
                distanceDir[0] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU, false)) {
                distanceDir[1] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            break;

        case DESTRA:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA, false)) { //Ultima direzione usata
                distanceDir[3] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SU, false)) {
                distanceDir[0] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU, false)) {
                distanceDir[1] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            break;
    }

    //Controllo tra le direzioni valide qual'e il percorso migliore per arrivare a pacman
    //Cerco la prima distanza valida e la imposto come minore
    for(i = 0; i < 4 && min == -1; i++)
        if(distanceDir[i] != -1)
            min = i;
    if(countDirValid > 1) {
        //Confronto a partire della posizione i tutte le altre distanze valide per trovare la minore
        while(i < 4) {
            if(distanceDir[i] < distanceDir[min] && distanceDir[i] != -1)
                min = i;
            i++;
        }
    }
    
    //Movimento effettivo fantasma 
    if(distanceDir[min] <= 16) {
        switch (min) {
            case 0: //SU
                ghost->lastDirValid = SU;
                ghost->p_pos.y -= 1;
                break;
            case 1: //GIU
                ghost->lastDirValid = GIU;
                ghost->p_pos.y += 1;
                break;
            case 2: //SX
                ghost->lastDirValid = SINISTRA;
                ghost->p_pos.x -= 2;
                break;
            case 3: //DX
                ghost->lastDirValid = DESTRA;
                ghost->p_pos.x += 2;
                break;
        }
        //Verifico se il fantasma è entrato nel tunnel
        if(ghost->p_pos.x < 0)
            ghost->p_pos.x = MAXX - 1;
        else if(ghost->p_pos.x >= MAXX)
            ghost->p_pos.x = 0;
    } else
        ghostRandom(ghost, map);

}

//Effettua in maniera randomica il prossimo passo del gohst
void ghostRandom(ghost_object* ghost, char map[MAXY][MAXX]) {
    int dirRandom;
    pos posGhost;
        
    posGhost = ghost->p_pos;
        
    //Se la sua ultima posizione valida è ancora valida continua in quel verso altrimenti ne genera un altro in maniera randomica
    if(!simulatePass(map, &posGhost, ghost->lastDirValid, false)) {
        do {
            posGhost = ghost->p_pos;
            
            //Genero in maniera random una direzione che non sia opposta rispetto a quella attuale
            do {
                dirRandom = rand() % 4;
            } while((dirRandom == 1 && ghost->lastDirValid == SU) || (dirRandom == 2 && ghost->lastDirValid == DESTRA) || (dirRandom == 0 && ghost->lastDirValid == GIU) || (dirRandom == 3 && ghost->lastDirValid == SINISTRA));  

            switch(dirRandom) {
                case 0: posGhost.y -= 1; break; //SU
                case 1: posGhost.y += 1; break; //GIU
                case 2: posGhost.x -= 2; break; //SX
                case 3: posGhost.x += 2; break; //DX
            }

            //Verifico se il fantasma è entrato nel tunnel
            if(posGhost.x < 0)
                posGhost.x = MAXX - 1;
            else if(posGhost.x >= MAXX)
                posGhost.x = 0;
        } while(!isValidSquare(map, posGhost.y, posGhost.x)); //Se nella posizione simulata c'è un muro rigenero una nuova posizione

        //Modifico l'ultima posizione valida con la nuova generata random
        switch(dirRandom) {
            case 0: ghost->lastDirValid = SU; break;
            case 1: ghost->lastDirValid = GIU; break;
            case 2: ghost->lastDirValid = SINISTRA; break;
            case 3: ghost->lastDirValid = DESTRA; break;
        }
    }
    
    //Movimento effettivo
    ghost->p_pos = posGhost;    
}

//Funzione aggiornamento ghosts, si occupa di gestire l'effetto rimbalzo e varie...
void updateGhost(gameState* game) {
    int i, j;

    for(i = 0; i < NUM_GHOST; i++) {
        if(game->ghostArray[i].isRead) {
            if(!game->ghostArray[i].isHome) { //Se il ghost non deve tornare nella home
                //Calcolo effetto rimbalzo
                reboundEffect(i, game->ghostArray, game->map);

                if(!game->ghostArray[i].insideHome) { //Se il ghost non è dentro la home
                    //Genero un numero da 0 a 30, se questo è 15 genero uno sparo
                    if(rand() % 31 == 15)
                        loadMissil(game, false, i);

                    //Se pacman ha mangiato una powerpill rendo i ghost vulnerabili
                    if(game->isEatPowerPill)
                        game->ghostArray[i].isVunerable = true;

                    //Se il ghost è morto genero un nuovo ghost che va nella home
                    if(game->ghostArray[i].isDead) {
                        game->ghostArray[i].countMissil = 0;
                        game->ghostArray[i].insideHome = false;
                        game->ghostArray[i].isDead = false;
                        game->ghostArray[i].isVunerable = false;
                        game->ghostArray[i].rebound = false;
                        startGhostThread(game, true, i);
                    }
                }
            }
            game->ghostArray[i].isWrite = true;
            game->ghostArray[i].isRead = false;
        }
    }

    //Disattivazione flag powerpill
    if(game->isEatPowerPill)
        game->isEatPowerPill = false;
}

//Funzione che permette a un ghost ritornare nella home
void goToHome(ghost_object* ghost, char map[MAXY][MAXX]) {
    pos posHome;
    int distanceDir[4] = { -1, -1, -1, -1 };
    int countDirValid = 0, min = -1, i;
    pos nextPos;
    
    //Coordinate casella d'ingresso della home
    posHome.x = 26;
    posHome.y = 13;
    
    //Controllo partendo dall'ultima direzione usata quali direzioni sono disponibili per il movimento
    switch(ghost->lastDirValid) {
        case SU:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SU, true)) { //Ultima direzione usata
                distanceDir[0] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA, true)) {
                distanceDir[3] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA, true)) {
                distanceDir[2] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            break;

        case GIU:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU, true)) { //Ultima direzione usata
                distanceDir[1] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA, true)) {
                distanceDir[3] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA, true)) {
                distanceDir[2] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            break;
        
        case SINISTRA:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA, true)) { //Ultima direzione usata
                distanceDir[2] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SU, true)) {
                distanceDir[0] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU, true)) {
                distanceDir[1] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            break;

        case DESTRA:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA, true)) { //Ultima direzione usata
                distanceDir[3] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SU, true)) {
                distanceDir[0] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU, true)) {
                distanceDir[1] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            break;
    }

    //Controllo tra le direzioni valide qual'e il percorso migliore per arrivare a pacman
    //Cerco la prima distanza valida e la imposto come minore
    for(i = 0; i < 4 && min == -1; i++)
        if(distanceDir[i] != -1)
            min = i;
    if(countDirValid > 1) {
        //Confronto a partire della posizione i tutte le altre distanze valide per trovare la minore
        while(i < 4) {
            if(distanceDir[i] < distanceDir[min] && distanceDir[i] != -1)
                min = i;
            i++;
        }
    }

    //Movimento effettivo
    switch (min) {
            case 0: //SU
                ghost->lastDirValid = SU;
                ghost->p_pos.y -= 1;
                break;
            case 1: //GIU
                ghost->lastDirValid = GIU;
                ghost->p_pos.y += 1;
                break;
            case 2: //SX
                ghost->lastDirValid = SINISTRA;
                ghost->p_pos.x -= 2;
                break;
            case 3: //DX
                ghost->lastDirValid = DESTRA;
                ghost->p_pos.x += 2;
                break;
        }
}

//Funzione che permette a un ghost di uscire dalla home
void goOutHome(ghost_object* ghost, char map[MAXY][MAXX]) {
    pos posHome;
    int distanceDir[4] = { -1, -1, -1, -1 };
    int countDirValid = 0, min = -1, i;
    pos nextPos;

    //Coordinate quadrato d'uscita della home
    posHome.x = 26;
    posHome.y = 11;
    
    //Controllo partendo dall'ultima direzione usata quali direzioni sono disponibili per il movimento
    switch(ghost->lastDirValid) {
        case SU:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SU, true)) { //Ultima direzione usata
                distanceDir[0] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA, true)) {
                distanceDir[3] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA, true)) {
                distanceDir[2] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            break;

        case GIU:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU, true)) { //Ultima direzione usata
                distanceDir[1] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA, true)) {
                distanceDir[3] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA, true)) {
                distanceDir[2] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            break;
        
        case SINISTRA:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA, true)) { //Ultima direzione usata
                distanceDir[2] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SU, true)) {
                distanceDir[0] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU, true)) {
                distanceDir[1] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            break;

        case DESTRA:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA, true)) { //Ultima direzione usata
                distanceDir[3] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SU, true)) {
                distanceDir[0] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU, true)) {
                distanceDir[1] = distanceToPacman(nextPos, posHome);
                countDirValid++;
            }
            break;
    }

    //Controllo tra le direzioni valide qual'e il percorso migliore per arrivare a pacman
    //Cerco la prima distanza valida e la imposto come minore
    for(i = 0; i < 4 && min == -1; i++)
        if(distanceDir[i] != -1)
            min = i;
    if(countDirValid > 1) {
        //Confronto a partire della posizione i tutte le altre distanze valide per trovare la minore
        while(i < 4) {
            if(distanceDir[i] < distanceDir[min] && distanceDir[i] != -1)
                min = i;
            i++;
        }
    }

    //Movimento effettivo
    switch (min) {
            case 0: //SU
                ghost->lastDirValid = SU;
                ghost->p_pos.y -= 1;
                break;
            case 1: //GIU
                ghost->lastDirValid = GIU;
                ghost->p_pos.y += 1;
                break;
            case 2: //SX
                ghost->lastDirValid = SINISTRA;
                ghost->p_pos.x -= 2;
                break;
            case 3: //DX
                ghost->lastDirValid = DESTRA;
                ghost->p_pos.x += 2;
                break;
        }
}

/* --- FUNZIONI SUPPORTO --- */

//Genera in maniera random la direzione iniziale di un ghost
void generaDirIniziale(ghost_object* ghost) {
    switch(rand() % 4) {
        case 0: ghost->lastDirValid = SU; break;
        case 1: ghost->lastDirValid = GIU; break;
        case 2: ghost->lastDirValid = SINISTRA; break;
        case 3: ghost->lastDirValid = DESTRA; break;
    }
}

//Funzione che genera coordinate random per la posizione iniziale dei fantasmi
pos generaCoordinate(char map[MAXY][MAXX], ghost_object* array, int id) {
    int i;
    pos p_pos;
    bool isUse;

    do {
        isUse = false;

        p_pos.x = 2 + (rand() % (MAXX - 2));
        p_pos.y = 1 + (rand() % (MAXY - 1));

        //Evita le coordinate della home
        if(p_pos.x >= 22 && p_pos.x <= 32 && p_pos.y >= 13 && p_pos.y <= 15)
            isUse = true;
        //Evita le coordinate dei riquadri vuoti a fine mappa
        if(((p_pos.x >= 0 && p_pos.x <= 8) || (p_pos.x >= 46 && p_pos.x <= 54)) && ((p_pos.y >= 10  && p_pos.y <= 12) || (p_pos.y >= 16 && p_pos.y <= 18)))
            isUse = true;
        //Evita le coordinate x dispari
        if(p_pos.x % 2 != 0)
            isUse = true;
        //Controlla se le coordinate del ghost coincidono con quelle di pacman
        if((p_pos.x == 26 && p_pos.y == 23) || (26 == p_pos.x - 2 && 23 == p_pos.y) || (26 == p_pos.x + 2 && 23 == p_pos.y) || 
            (26 == p_pos.x && 23 == p_pos.y - 1) || (26 == p_pos.x && 23 == p_pos.y + 1))
            isUse = true;
        //Controllo se le coordinate generate sono state assegnate gia ad un altro ghost o se sono stati generati due fantasmi attaccati
        for(i = 0; i < id && !isUse; i++)
            if((array[i].p_pos.x == p_pos.x && array[i].p_pos.y == p_pos.y) || (array[i].p_pos.x == p_pos.x - 2 && array[i].p_pos.y == p_pos.y) ||
                (array[i].p_pos.x == p_pos.x + 2 && array[i].p_pos.y == p_pos.y) || (array[i].p_pos.x == p_pos.x && array[i].p_pos.y == p_pos.y - 1) ||
                (array[i].p_pos.x == p_pos.x && array[i].p_pos.y == p_pos.y + 1))
                isUse = true;

    } while((getSquareMap(map, p_pos.y, p_pos.x) != ' ') || isUse);

    return p_pos;
} 

//Simula il passo successivo di un ghost in una data direzione e controlla se vi è un muro oppure no
//param isHome => indica se durante la simulazione si devono considerare solo i muri oppura anche la porta della home 
bool simulatePass(char map[MAXY][MAXX], pos* p_pos, int dir, bool isHome) {
    switch(dir) {
        case SU:
            p_pos->y -= 1;
            break;
        case GIU:
            p_pos->y += 1;
            break;
        case SINISTRA:
            p_pos->x -= 2;
            break;
        case DESTRA:
            p_pos->x += 2;
            break;
    }

    //Controllo se il fantasma è entrato nel tunnel
    if(p_pos->x < 0)
        p_pos->x = MAXX - 1;
    else if(p_pos->x >= MAXX)
        p_pos->x = 0;
    
    if((getSquareMap(map, p_pos->y, p_pos->x) != '0' && isHome) || (isValidSquare(map, p_pos->y, p_pos->x) && !isHome))
        return true;
    return false;
}

//Calcola la distanza che vi è tra un ghost e pacman
float distanceToPacman(pos posGhost, pos posPacman) {
    float distance;
    int x, y;

    //Ottengo la lunghezza dei due cateti
    x = abs(posGhost.x - posPacman.x);
    y = abs(posGhost.y - posPacman.y);

    if(x != 0 && y !=0)
        //Calcolo il teorema di pitagora per ottenere l'ipotenusa ovvero la distanza tra pacman e il ghost
        distance = sqrt(pow(x, 2) + pow(y, 2));
    else if(y != 0)
        distance = y;
    else
        distance = x;
    return distance;
}

//Calcola l'effetto rimbalzo di un singolo ghost con tutti gli altri ghost
void reboundEffect(int index, ghost_object array[], char map[MAXY][MAXX]) {
    int i;
    
    for(i = 0 ;i < NUM_GHOST; i++)
        if(i != index && !array[i].isHome) {    //Controlla se il ghost analizzato e diverso rispetto a quello passato come parametro
            if(array[index].p_pos.x == array[i].p_pos.x && array[index].p_pos.y == array[i].p_pos.y) {
                inverseDir(&array[index], false, map);
                inverseDir(&array[i], false, map);
            } else {
                //Per evitare scambi tra ghosts controllo se nella casella successiva della direzione del ghost vi è un altro ghost 
                switch(array[index].lastDirValid) {
                    case SINISTRA:
                        if(array[index].p_pos.x - 2 == array[i].p_pos.x && array[index].p_pos.y == array[i].p_pos.y) {
                            inverseDir(&array[index], true, map);
                            inverseDir(&array[i], true, map);
                        }
                        break;
                    case DESTRA:
                        if(array[index].p_pos.x + 2 == array[i].p_pos.x && array[index].p_pos.y == array[i].p_pos.y) {
                            inverseDir(&array[index], true, map);
                            inverseDir(&array[i], true, map);
                        }
                        break;
                    case GIU:
                        if(array[index].p_pos.y + 1 == array[i].p_pos.y && array[index].p_pos.x == array[i].p_pos.x) {
                            inverseDir(&array[index], true, map);
                            inverseDir(&array[i], true, map);
                        }        
                        break;
                    case SU:
                        if(array[index].p_pos.y - 1 == array[i].p_pos.y && array[index].p_pos.x == array[i].p_pos.x) {
                            inverseDir(&array[index], true, map);
                            inverseDir(&array[i], true, map);
                        }
                        break;
                }
            }
        }
}

//Vine chiamata nel caso il ghost a causa del muro non puo invertire la traiettoria dopo un rimbalzo
void reboundPass(ghost_object* ghost, char map[MAXY][MAXX]) {
    pos posGhost = ghost->p_pos;
    
    //Se la sx e dx è bloccata simulo su o giu e viceversa
    switch (ghost->lastDirValid) {
        case SINISTRA:
        case DESTRA:
            if(simulatePass(map, &posGhost, SU, false))
                ghost->lastDirValid = SU;
            else {
                posGhost = ghost->p_pos;
                simulatePass(map, &posGhost, GIU, false);
                ghost->lastDirValid = GIU;
            }
            ghost->p_pos = posGhost;
            break;
        case SU:
        case GIU:
            if(simulatePass(map, &posGhost, SINISTRA, false))
                ghost->lastDirValid = SINISTRA;
            else {
                posGhost = ghost->p_pos;
                simulatePass(map, &posGhost, DESTRA, false);
                ghost->lastDirValid = DESTRA;
            }
            ghost->p_pos = posGhost;
            break;
    }
}

//Calcola la direzione inversa che il ghost deve eseguire
//param onlyDir => indica se si deve invertire solo la direzione o anche le coordinate
void inverseDir(ghost_object* ghost, bool onlyDir, char map[MAXY][MAXX]) {
    switch (ghost->lastDirValid) {
        case SU: 
            if(!onlyDir) {
                if(getSquareMap(map, ghost->p_pos.y + 1, ghost->p_pos.x) != '0') {
                    ghost->lastDirValid = GIU; 
                    ghost->p_pos.y += 1;
                } else 
                    reboundPass(ghost, map);
            } else {
                ghost->lastDirValid = GIU; 
                ghost->rebound = true;
            }
            break;
        case GIU:    
            if(!onlyDir) {
                if(getSquareMap(map, ghost->p_pos.y - 1, ghost->p_pos.x) != '0') {
                    ghost->lastDirValid = SU; 
                    ghost->p_pos.y -= 1;
                } else
                    reboundPass(ghost, map);                
            } else {
                ghost->lastDirValid = SU;
                ghost->rebound = true;
            }
            break;
        case SINISTRA:
            if(!onlyDir) {
                if(getSquareMap(map, ghost->p_pos.y, ghost->p_pos.x + 2) != '0') {
                    ghost->lastDirValid = DESTRA;
                    ghost->p_pos.x += 2;
                } else
                    reboundPass(ghost, map);                
            } else {
                ghost->lastDirValid = DESTRA; 
                ghost->rebound = true;
            }
            break;
        case DESTRA:
            if(!onlyDir) {
                if(getSquareMap(map, ghost->p_pos.y, ghost->p_pos.x - 2) != '0') {
                    ghost->lastDirValid = SINISTRA; 
                    ghost->p_pos.x -= 2; 
                } else
                    reboundPass(ghost, map);               
            } else {
                ghost->lastDirValid = SINISTRA; 
                ghost->rebound = true;
            }
            break;
    }
}