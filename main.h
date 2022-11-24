#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "util.h"
#define TRUE 1
#define FALSE 0
#define SEC_IN_STATE 1

#define ROOT 0

#define maxi(a, b) (((a) > (b)) ? (a) : (b))

extern int rank, size,
            lampClock, priorytet, sekundanci, saleSzpitalne,
            aggrementSum, rival, bufer[];
typedef enum {InRun, InSend, InMonitor, InFree, ToHeal} state_t;
extern state_t stan;
typedef enum {Heal, Sekundant, Fight, Rival} state_f;
extern state_f stan_od;
extern pthread_t threadKom, threadMon;

extern pthread_mutex_t stateMut, stateForMut, lampMut,
                        priorityMut, aggrementSumMut, buferMut;

#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#define println(FORMAT,...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);
#endif
