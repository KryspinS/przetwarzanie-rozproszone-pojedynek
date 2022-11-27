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
    {"zwolnienie zasobu", FREE} 
};

struct stateNames_t{
    const char *name;
    state_f state;
} stateNames[] = {
    {"leczenia", Heal},
    {"sekundanta", Sekundant},
    {"walki", Fight},
    {"rywala", Rival}   
};

const char const *state2string( state_f state )
{
    for (int i=0; i <sizeof(stateNames)/sizeof(struct stateNames_t);i++) {
	if ( stateNames[i].state == state )  return stateNames[i].name;
    }
    return "<unknown>";
}

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
    int       blocklengths[NITEMS] = {1,1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);
    offsets[3] = offsetof(packet_t, value);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

void sendPacket(packet_t *pkt, int destination, int tag)
{
    pkt->src = rank;
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    increaseLamport(0);
    debug("Wysyłam %s do %d na temat %s\n", tag2string( tag), destination, state2string(pkt->data));
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

void setRivals(int who, int value, int type)
{
    pthread_mutex_lock( &rivalsMut );
    list_t list = {who, value, type};
    rivalsList[who] = list;
    pthread_mutex_unlock( &rivalsMut );
}

void swap(list_t *xp, list_t *yp)
{
    list_t temp = *xp;
    *xp = *yp;
    *yp = temp;
}
 
void sortAndChooseRival()
{
    pthread_mutex_lock( &rivalsMut );
    int i, j, min_idx;
 
    for (i = 0; i < size-1; i++)
    {
        min_idx = i;
        for (j = i+1; j < size; j++)
          if (rivalsList[j].value < rivalsList[min_idx].value)
            min_idx = j;
 
           if(min_idx != i)
            swap(&rivalsList[min_idx], &rivalsList[i]);
    }
    
    chooseRival();
    pthread_mutex_unlock( &rivalsMut );
}

void chooseRival() 
{
    int counter;
    for (int i=0; i <size;i++)
    {
        if(rivalsList[i].msg == NACK) continue;
        counter++;
        if (rivalsList[i].id == rank)
        {
            if (counter%2 != 0)
            {
                if (i != size-1)
                {
                    rival = rivalsList[i+1].id;
                }
                else 
                {
                    increaseAggrementSum(0);
                    changeState(InMonitor);
                    setRivals(rank, rivalsList[i].value, ACK);
                    break;
                }
            }
            else 
            {
                rival = rivalsList[i-1].id;
            }
        }
        if (i == size-1 && counter%2 != 0)
        {
            setRivals(rivalsList[i].id, rivalsList[i].value, ACK);
        }
    }
}