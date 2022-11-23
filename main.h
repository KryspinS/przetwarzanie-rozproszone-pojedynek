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
#define STATE_CHANGE_PROB 10

#define ROOT 0

#define RIVAL 0
#define SEKUNDANT 1
#define FIGHT 2
#define HEAL 3
#define maxi(a, b) (((a) > (b)) ? (a) : (b))

extern int rank;
extern int size;
extern int lampClock;
extern int sekundanci;
extern int saleSzpitalne;
extern int aggrementSum;
extern int rival;
typedef enum {InRun, InHealSend, InFightSend, InSecundantSend, InHealMonitor, InFightMonitor, InSecundantMonitor, InFree, ToHeal} state_t;
extern state_t stan;
extern pthread_t threadKom, threadMon;

extern pthread_mutex_t stateMut;
extern pthread_mutex_t lampMut;

#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#define println(FORMAT,...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);

void changeState( state_t );

#endif
