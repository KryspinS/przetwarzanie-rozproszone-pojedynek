#ifndef UTILH
#define UTILH
#include "main.h"

extern MPI_Datatype MPI_PAKIET_T;

const char const *state2string( state_f state );
const char const *tag2string( int tag );


void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);
void changeState( state_t newState );
void changeStateFor( state_f newStatef );
void increaseLamport(int requestedLamportClock) ;
void setPriority();
void increaseAggrementSum(int newValue);
void setBufer(int who, int value);
void setRivals(int who, int value, int type);
void swap(list_t *xp, list_t *yp);
void sortAndChooseRival();
void chooseRival();
#endif
