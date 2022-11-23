#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while (1) {
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        switch ( pakiet.data ) {
            case RIVAL:
                
                break;
            case SEKUNDANT: 
            case HEAL:
                if (stan == InHealMonitor || stan == InSecundantMonitor)
                {
                
                }
                break;
            case FIGHT:
                
                break;
            
        }
        switch ( status.MPI_TAG ) {
            case REQ:
                break;
            case ACK:
                break;
            case FREE:
                break;
            case NACK:
                break;
        }
        
    }
}
