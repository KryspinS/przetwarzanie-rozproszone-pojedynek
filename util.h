#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;  
    state_f data;
    int value;
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3

/* Typy wiadomości */
#define REQ 0
#define ACK 1
#define FREE 2
#define NACK -1


extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);
void changeState( state_t newState );
void changeStateFor( state_f newStatef );
void increaseLamport(int requestedLamportClock) ;
void setPriority();
void increaseAggrementSum(int newValue);
void setBufer(int who, int value);
#endif
