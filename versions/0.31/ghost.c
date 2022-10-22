#include <pthread.h>
#include <ncurses.h>
#include <unistd.h>
#include "game.h"
#include "area.h"
#include <mm_malloc.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

/* --- DATI INTERNI --- */
typedef struct {
    ghost_object* ghost;
    pos* posPacman;
    char (*map)[MAXY][MAXX]; //Puntatore a matrice
    difficulty mode;
} argsThread;

/* --- DICHIARAZIONE FUNZIONI INTERNE --- */
pos generaCoordinate(char map[MAXY][MAXX], ghost_object* array, int id);
bool simulatePass(char map[MAXY][MAXX], pos* p_pos, int dir);
void* ghostRoutine(void* param);
float distanceToPacman(pos posGhost, pos posPacman);
void ghostAI(ghost_object* ghost, pos posPacman, char map[MAXY][MAXX]);
void ghostRandom(ghost_object* ghost, char map[MAXY][MAXX]);
void reboundEffect(int index, ghost_object* array);
void inverseDir(ghost_object* ghost, bool onlyDir);

/* --- IMPLEMENTAZIONE FUNZIONI --- */

//Inizializza tutti i fantasmi
void inizialiseGhosts(gameState* game) {
    int i;
    
    game->ghostArray = (ghost_object*) malloc(sizeof(ghost_object) * NUM_GHOST);

    for(i = 0; i < NUM_GHOST; i++){
        game->ghostArray[i].p_pos = generaCoordinate(game->map, game->ghostArray, i);
        game->ghostArray[i].lives = 10; 
        game->ghostArray[i].lastDirValid = SINISTRA;
        game->ghostArray[i].isWrite = true;
        game->ghostArray[i].isRead = false;
        game->ghostArray[i].isVunerable = false;
        game->ghostArray[i].isHome = false;

        switch(game->mode) {
            case EASY:
            case MEDIUM: game->ghostArray[i].speed = 2; break;
            case HARD: game->ghostArray[i].speed = 1; break;
        }
    }
}

//Rinizializza tutti i fantasmi
void reinizialiseGhosts(gameState* game) {
    int i = 0;
    argsThread* args = NULL;

    //Attendo la terminazione di tutti i thread fantasma
    for(i = 0; i < NUM_GHOST; i++)
        game->ghostArray[i].lives = 0;

    inizialiseGhosts(game);
}

//
void startGhostsThread(gameState* game) {
    int i;
    argsThread* args = NULL;

    for(i = 0; i < NUM_GHOST; i++) {
        //Caricamento parametri thread
        args = (argsThread*) malloc(sizeof(argsThread));
        args->ghost = &game->ghostArray[i];
        args->posPacman = &game->pacman.p_pos;
        args->map = &game->map;
        args->mode = game->mode;

        //Creazione thread
        pthread_create(&game->ghostArray[i].ghostID, NULL, &ghostRoutine, args);
    }
}

//Funzione thread ghost
void* ghostRoutine(void* param) {
    argsThread* args = (argsThread*) param;
    bool startCount = false;
    time_t t;
    int sleep;

    if(!args->ghost->isHome) {
        sleep = (1 + rand() % 3) * 1000000;
        usleep(sleep);  //Tempo di attesa random da 1 a 3 secondi prima che il ghost inizi a muoversi
    }
    while(args->ghost->lives > 0){
        pthread_mutex_lock(&rw_mutex);
        
        if(args->ghost->isWrite) {
            if(args->ghost->speed-- == 0) {
                switch(args->mode) {
                    case EASY:
                    case MEDIUM: args->ghost->speed = 2; break;
                    case HARD: args->ghost->speed = 1; break;
                }
                
                if(args->ghost->isVunerable && !startCount) {
                        t = time(NULL);
                        startCount = true;
                }

                //Movimento ghost
                switch(args->mode) {
                    case EASY:
                        ghostRandom(args->ghost, *args->map);
                        break;
                    case MEDIUM:
                    case HARD:
                        if(args->ghost->isVunerable) 
                            ghostRandom(args->ghost, *args->map);
                        else
                            ghostAI(args->ghost, *args->posPacman, *args->map);
                }

                if((time(NULL) - t) > 10 && args->ghost->isVunerable) {
                    args->ghost->isVunerable = false;
                    startCount = false;
                }

            }
            args->ghost->isWrite = false;
            args->ghost->isRead = true;
        }
        
        pthread_mutex_unlock(&rw_mutex);
        usleep(124000);
    }
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
            if(simulatePass(map, &nextPos, SU)) { //Ultima direzione usata
                distanceDir[0] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA)) {
                distanceDir[3] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA)) {
                distanceDir[2] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            break;

        case GIU:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU)) { //Ultima direzione usata
                distanceDir[1] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA)) {
                distanceDir[3] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA)) {
                distanceDir[2] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            break;
        
        case SINISTRA:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SINISTRA)) { //Ultima direzione usata
                distanceDir[2] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SU)) {
                distanceDir[0] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU)) {
                distanceDir[1] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            break;

        case DESTRA:
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, DESTRA)) { //Ultima direzione usata
                distanceDir[3] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }

            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, SU)) {
                distanceDir[0] = distanceToPacman(nextPos, posPacman);
                countDirValid++;
            }
            
            nextPos = ghost->p_pos;
            if(simulatePass(map, &nextPos, GIU)) {
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
    
    if(distanceDir[min] <= 16) {
        //Movimento effettivo fantasma 
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
    if(!simulatePass(map, &posGhost, ghost->lastDirValid)) {
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

//
void updateGhost(gameState* game) {
    int i, j;

    for(i = 0; i < NUM_GHOST; i++) {
        if(game->ghostArray[i].isRead) {
            reboundEffect(i, game->ghostArray);
            
            if(game->isEatPowerPill) {
                game->ghostArray[i].isVunerable = true;
                if(i == NUM_GHOST - 1)
                game->isEatPowerPill = false;
            }

            game->ghostArray[i].isWrite = true;
            game->ghostArray[i].isRead = false;
        }
    }
}

/* --- FUNZIONI SUPPORTO --- */

//Funzione che genera coordinate random per la posizione iniziale dei fantasmi
pos generaCoordinate(char map[MAXY][MAXX], ghost_object* array, int id) {
    int i;
    pos p_pos;
    bool isUse;

    do {
        isUse = false;

        p_pos.x = 2 + (rand() % (MAXX - 2));
        p_pos.y = 1 + (rand() % (MAXY - 1));

        //Controllo se le coordinate generate sono state assegnate gia ad un altro ghost o se sono stati generati due fantasmi attaccati
        for(i = 0; i < id; i++)
            if((array[i].p_pos.x == p_pos.x && array[i].p_pos.y == p_pos.y) ||
                (array[i].p_pos.x == p_pos.x - 2 && array[i].p_pos.y == p_pos.y) ||
                (array[i].p_pos.x == p_pos.x + 2 && array[i].p_pos.y == p_pos.y) ||
                (array[i].p_pos.x == p_pos.x && array[i].p_pos.y == p_pos.y - 1) ||
                (array[i].p_pos.x == p_pos.x && array[i].p_pos.y == p_pos.y + 1))
                isUse = true;

    } while((getSquareMap(map, p_pos.y, p_pos.x) != '~') || isUse);

    return p_pos;
} 

//Simula il passo successivo di un ghost in una data direzione e controlla se vi è un muro oppure no
bool simulatePass(char map[MAXY][MAXX], pos* p_pos, int dir) {
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
    
    if(isValidSquare(map, p_pos->y, p_pos->x))
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
void reboundEffect(int index, ghost_object array[]) {
    int i;
    
    for(i = 0 ; i < NUM_GHOST; i++)
        if(i != index) {    //Controlla se il ghost analizzato e diverso rispetto a quello passato come parametro
            if(array[index].p_pos.x == array[i].p_pos.x && array[index].p_pos.y == array[i].p_pos.y) {
                inverseDir(&array[index], false);
                inverseDir(&array[i], false);
            }

            switch(array[index].lastDirValid) {
                case SINISTRA:
                    if(array[index].p_pos.x - 2 == array[i].p_pos.x && array[index].p_pos.y == array[i].p_pos.y) {
                        inverseDir(&array[index], true);
                        inverseDir(&array[i], true);
                    }
                    break;

                case DESTRA:
                    if(array[index].p_pos.x + 2 == array[i].p_pos.x && array[index].p_pos.y == array[i].p_pos.y) {
                        inverseDir(&array[index], true);
                        inverseDir(&array[i], true);
                    }
                    break;

                case GIU:
                    if(array[index].p_pos.y + 1 == array[i].p_pos.y && array[index].p_pos.x == array[i].p_pos.x) {
                        inverseDir(&array[index], true);
                        inverseDir(&array[i], true);
                    }        
                    break;
                    
                case SU:
                    if(array[index].p_pos.y - 1 == array[i].p_pos.y && array[index].p_pos.x == array[i].p_pos.x) {
                        inverseDir(&array[index], true);
                        inverseDir(&array[i], true);
                    }
                    break;
            }
        }
}

//Calcola la direzione inversa che il ghost deve eseguire
//param onlyDir => indica se si deve invertire solo la direzione o anche le coordinate
void inverseDir(ghost_object* ghost, bool onlyDir) {
    switch (ghost->lastDirValid) {
        case SU: 
            ghost->lastDirValid = GIU; 
            if(!onlyDir)
                ghost->p_pos.y += 1;
            break;
        case GIU:    
            ghost->lastDirValid = SU; 
            if(!onlyDir)
                ghost->p_pos.y -= 1;
            break;
        case SINISTRA:
            ghost->lastDirValid = DESTRA; 
            if(!onlyDir)
                ghost->p_pos.x += 2;
            break;
        case DESTRA:
            ghost->lastDirValid = SINISTRA; 
            if(!onlyDir)
                ghost->p_pos.x -= 2;    
            break;
    }
}