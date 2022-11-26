#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet, odp;
    while (1) {
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        increaseLamport(pakiet.ts);

        switch ( status.MPI_TAG ) {
            case REQ:
                    odp.data = pakiet.data;
                    if (stan == InMonitor)
                    {
                        switch ( pakiet.data ) {
                            case Rival:
                                increaseAggrementSum(1);
                                setRivals(pakiet.src, pakiet.ts, ACK);
                                break;
                            case Sekundant: 
                            case Heal:
                                if(stan_od == pakiet.data)
                                {
                                    if(priorytet > pakiet.ts || (priorytet == pakiet.ts && pakiet.src < rank))
                                    {
                                        increaseAggrementSum(1);
                                        setBufer(pakiet.src, TRUE);
                                        break;
                                    }
                                }
                                sendPacket(&odp, pakiet.src, ACK);
                                break;
                            case Fight:
                                increaseAggrementSum(pakiet.value);
                                break;
                        }
                    }
                    else if (pakiet.data == Rival)
                    {
                        sendPacket(&odp, pakiet.src, NACK);
                    }
                    else
                    //Jeśli nie monitoruję to wysyłam ACK
                    {
                        sendPacket(&odp, pakiet.src, ACK);
                    }
                break;
            case FREE:
            case ACK:
            case NACK:
                increaseAggrementSum(1);
                if(pakiet.data = Rival)
                {
                    setRivals(pakiet.src, pakiet.ts, status.MPI_TAG);
                }
                break;
        }        
    }
}
