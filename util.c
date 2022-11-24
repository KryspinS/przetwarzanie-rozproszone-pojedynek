#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

struct tagNames_t{
    const char *name;
    int tag;
} tagNames[] = { 
    {"request", REQ }, 
    {"akceptacja", ACK}, 
    {"brak akceptacji", NACK}, 
    {"zwolnienie zasobu", FREE}};

const char const *tag2string( int tag )
{
    for (int i=0; i <sizeof(tagNames)/sizeof(struct tagNames_t);i++) {
	if ( tagNames[i].tag == tag )  return tagNames[i].name;
    }
    return "<unknown>";
}
/* tworzy typ MPI_PAKIET_T
*/
void inicjuj_typ_pakietu()
{
    int       blocklengths[NITEMS] = {1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

void sendPacket(packet_t *pkt, int destination, int tag)
{
    if (pkt==0) pkt = malloc(sizeof(packet_t));
    pkt->src = rank;
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    increaseLamport(0);
    debug("Wysyłam %s do %d\n", tag2string( tag), destination);
    free(pkt);
}

void changeState( state_t newState )
{
    pthread_mutex_lock( &stateMut );
    stan = newState;
    pthread_mutex_unlock( &stateMut );
}

void changeStateFor( state_f newStatef )
{
    pthread_mutex_lock( &stateForMut );
    stan_od = newStatef;
    pthread_mutex_unlock( &stateForMut );
}

void increaseLamport(int requestedLamportClock) 
{
    pthread_mutex_lock( &lampMut );
    if (requestedLamportClock == 0) { lampClock++; }
    else {lampClock = maxi(lampClock, requestedLamportClock)+1;}
    pthread_mutex_unlock( &lampMut );
}

void setPriority()
{
    pthread_mutex_lock( &priorityMut );
    pthread_mutex_lock( &lampMut );
    priorytet = lampClock;
    pthread_mutex_unlock( &lampMut );
    pthread_mutex_unlock( &priorityMut );
}

void increaseAggrementSum(int newValue)
{
    pthread_mutex_lock( &aggrementSumMut );
    aggrementSum = newValue == 0 ? 0 : aggrementSum+newValue;
    pthread_mutex_unlock( &aggrementSumMut );

}

void setBufer(int who, int value)
{
    pthread_mutex_lock( &buferMut );
    bufer[who] = value;
    pthread_mutex_unlock( &buferMut );
}