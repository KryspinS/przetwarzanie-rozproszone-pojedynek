#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define ROOT 0
#define MSG_TAG 100
#define maxi(a,b) (((a)>(b))?(a):(b))



int CommunicationController(int *debug, int *tid, int *size, int *LampClock,
                            int *msg, int *buf)
{
    MPI_Status status;
    int ack = 0;
    int stocType = msg[1];
    int* reqList = (int*)calloc(*size, sizeof(int));

    for (int i=0;i<*size;i++)
    {
        if (i == *tid) {continue;}
        msg[0] = ++*LampClock;
        msg[2] = 0; //0-REQ, 1-ACK
        MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD );
        if(*debug) printf("[DEBUG][SEND][%d] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", stocType, *tid, *LampClock, i, msg[0], msg[1], msg[2]);
        reqList[i] = *LampClock;
    }
        if(*debug) printf("[DEBUG][SEND][%d] [%d]:[%d] Dane: REQ Priority: 0:%d 1:%d 2:%d 3:%d \n", stocType, *tid, *LampClock, reqList[0], reqList[1], reqList[2], reqList[3]);

    do
    {
        MPI_Recv(msg, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(*debug) printf("[DEBUG][RECEIVE][%d] [%d]:[%d] Dane: OD:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", stocType, *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);

        *LampClock = maxi(*LampClock, msg[0]) + 1;
        if(msg[2] == 1) //ACK
        {
            ack++;
            buf[status.MPI_SOURCE] = 0;
        }
        else if(msg[2] == 0) //REQ
        {
            if (msg[1] == stocType) //jesli proces 'i' ubiega sie o ten sam zasob 
            {
                if (msg[0] > reqList[status.MPI_SOURCE]) // 'i' ma wiekszy priorytet to ACK
                {
                    msg[0] = ++*LampClock;
                    msg[1] = stocType;
                    msg[2] = 1;
                    MPI_Send(msg, 3, MPI_INT, status.MPI_SOURCE, MSG_TAG, MPI_COMM_WORLD );
                    if(*debug) printf("[DEBUG][SEND][%d] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", stocType, *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);
        
                }
                else
                {
                    if (msg[0] == reqList[status.MPI_SOURCE] && *tid < status.MPI_SOURCE) // 'i' ma jednakowy priorytet, ale wyzszy nr procesu - ACK
                    {
                        msg[0] = ++*LampClock;
                        msg[1] = stocType;
                        msg[2] = 1;
                        MPI_Send(msg, 3, MPI_INT, status.MPI_SOURCE, MSG_TAG, MPI_COMM_WORLD );
                        if(*debug) printf("[DEBUG][SEND][%d] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", stocType, *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);
                    } else // 'i' ma mniejszy priorytet lub nizszy nr procesu trafa na kolejke oczekujacych
                    {
                        buf[status.MPI_SOURCE] = 1;
                    }
                }
                
            }
            else //jesli proces 'i' ubiega sie o inny zasob to ACK
            {
                msg[0] = ++*LampClock;
                msg[1] = stocType;
                msg[2] = 1;
                MPI_Send(msg, 3, MPI_INT, status.MPI_SOURCE, MSG_TAG, MPI_COMM_WORLD );
                if(*debug) printf("[DEBUG][SEND][%d] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", stocType, *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);

            } 
        }
    } while (ack < *size-1);
    return ack;
}

int SearchForRival(int *debug, int *tid, int *size, int *LampClock, int *msg) {
    MPI_Status status;
    int rival = -1;
    printf("[%d]:[%d] Szukam przeciwnika\n", *tid, *LampClock);

    for(int i=0; i<*size; i++){
        if (i == *tid) {continue;}
        msg[0] = ++*LampClock;
        msg[1] = 0; //0 = Szuka rywala
        msg[2] = 0;
        MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD );
        if(*debug) printf("[DEBUG][SEND][0] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", *tid, *LampClock, i, msg[0], msg[1], msg[2]);
    
    }

    for(int i=0; i<*size; i++){
        if (i == *tid) {continue;}
        MPI_Recv(msg, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(*debug) printf("[DEBUG][RECEIVE][0] [%d]:[%d] Dane: OD:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);

        *LampClock = maxi(*LampClock, msg[0]) + 1;
        
        if (*LampClock > msg[0] && msg[1] == 0 && rival == -1) {
            MPI_Send(msg, 3, MPI_INT, status.MPI_SOURCE, MSG_TAG, MPI_COMM_WORLD );
            MPI_Recv(msg, 3, MPI_INT, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (msg[1] == 0) {
                printf("[%d]:[%d] Wezwany na pojedynek z %d\n", *tid, *LampClock, status.MPI_SOURCE);
                rival = status.MPI_SOURCE;
            }
        }
    }
    if(rival != -1) {
        return rival;
    }
    
    printf("[%d]:[%d] Nie znalazem rywala, szukam dalej!\n", *tid, *LampClock);
    return -1;
}

int SearchForSecundant(int *debug, int *tid, int *size, int *LampClock, 
                        int *msg, int *Sekundanci, int *buf) {
    MPI_Status status;
    int ack;
    msg[1] = 1; //1 = Szuka Sekundanta
    printf("[%d]:[%d] Szukam Sekundanta\n", *tid, *LampClock);    

    ack = CommunicationController(debug, tid, size, LampClock, msg, buf);

    int sum = *Sekundanci - (*size - 1 - ack);
    if(*debug) printf("[DEBUG][SearchForSecundant] [%d]:[%d] Dane ODEBRANE: SUM:[%d] \n", *tid, *LampClock, sum);
    if (sum > 0) {
        return 1;
    }
    
    printf("[%d]:[%d] Nie znalazem Sekundanata, probuje jeszcze raz!\n", *tid, *LampClock);            
    return 0;
}

int Fight(int *debug, int *tid, int *size, int *LampClock, int *msg, int *rival, int *buf) {
    struct timespec spec;
    MPI_Status status;
    int res, shot;
    msg[1] = 2; //2 = walczy
    printf("[%d]:[%d] Walcze z %d\n", *tid, *LampClock, *rival); 
    clock_gettime(CLOCK_REALTIME, &spec);
    srand(spec.tv_nsec);
    shot = rand()%1000 + 1; //ten konflikt zasluguje na losowosc dlatego nie Lamport

    msg[0] = ++*LampClock;
    msg[2] = shot; 
    MPI_Send(msg, 3, MPI_INT, *rival, MSG_TAG, MPI_COMM_WORLD );
    if(*debug) printf("[DEBUG][SEND][2] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", *tid, *LampClock, *rival, msg[0], msg[1], msg[2]);
    MPI_Recv(msg, 3, MPI_INT, *rival, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(*debug) printf("[DEBUG][RECEIVE][2] [%d]:[%d] Dane: OD:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);
    *LampClock = maxi(*LampClock, msg[0]) + 1;
        
    if (msg[2] < shot && msg[1] == 2) {
        printf("[%d]:[%d] Wygralem z %d\n", *tid, *LampClock, *rival);
        res = 1;    
    } else if (msg[2] > shot && msg[1] == 2) {
        printf("[%d]:[%d] Poleglem z %d\n", *tid, *LampClock, *rival);    
        res = 0;    
    } else {
        printf("[%d]:[%d] Walcze nadal z %d!\n", *tid, *LampClock, *rival);            
        res = -1;
    }

    if(res != -1) {
        for (int i=0;i<*size;i++) {
            if (i == *tid) {continue;}
            if (buf[i]) {
                msg[0] = ++*LampClock;
                msg[1] = 2;
                msg[2] = 1;
                MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD );
                if(*debug) printf("[DEBUG][SEND][2] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", *tid, *LampClock, i, msg[0], msg[1], msg[2]);

            }
        }
    }

    return res;

}

int HealMe(int *debug, int *tid, int *size, int *LampClock, int *msg, int *SaleSzpitalne, int *buf) {
    struct timespec spec;
    MPI_Status status;
    int ack, nurseSpeed;
    msg[1] = 3; //3 = Szuka Sali szpitalnej
    printf("[%d]:[%d] Szukam wolnego lozka\n", *tid, *LampClock);    

    ack = CommunicationController(debug, tid, size, LampClock, msg, buf);
    int sum = *SaleSzpitalne - (*size - 1 - ack);
    
    if (sum > 0) {
        clock_gettime(CLOCK_REALTIME, &spec);
        srand(spec.tv_nsec);
        nurseSpeed = rand()%10 + 1;
        
        printf("[%d]:[%d] Znalazlem lozko, bede leczyl sie %ds\n",*tid, *LampClock, nurseSpeed);    
        sleep(nurseSpeed);
        
        printf("[%d]:[%d] Wyleczony\n",*tid, *LampClock);    
        for (int i=0;i<*size;i++) {
            if (i == *tid) {continue;}
            if (buf[i]) {
                msg[0] = ++*LampClock;
                msg[1] = 3;
                msg[2] = 1;
                MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD );
                if(*debug) printf("[DEBUG][SEND][3] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", *tid, *LampClock, i, msg[0], msg[1], msg[2]);

            }
        }
        
        return 1;
    }
    
    printf("[%d]:[%d] Nadal szukam lozka w szpitalu!\n", *tid, *LampClock);   
    return 0;

}

int main(int argc, char **argv)
{
	int tid, size, msg[3],
        Sekundanci, SaleSzpitalne, LampClock,
        rival, score, sekundant, heal, debug;
    int* SekundanciBuffer;
    int* SaleSzpitalneBuffer;
    Sekundanci = atoi(argv[1]);
    SaleSzpitalne = atoi(argv[2]);
    debug = atoi(argv[3]);
    LampClock = 0;

    if(Sekundanci < 2) {
        printf("[%d]:[%d] Sekundantow musi byc conajmniej dwoch, a podano: %d\n", tid, LampClock, Sekundanci);   
        exit(0); 
    }
    if(SaleSzpitalne < 1) {
        printf("[%d]:[%d] Sal szpitalnych musi byc conajmniej jedna, a podano: %d\n", tid, LampClock, SaleSzpitalne);    
        exit(0); 
    }

	MPI_Init(&argc, &argv);
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Comm_rank( MPI_COMM_WORLD, &tid );

    SekundanciBuffer = (int*) calloc(size, sizeof(int));
    SaleSzpitalneBuffer = (int*) calloc(size, sizeof(int));


	printf("[%d]:[%d] Witam!\n", tid, LampClock);
    while(1) {
        do {
            rival = SearchForRival(&debug, &tid, &size, &LampClock, &msg[2]);
            fflush(stdout);
            sleep(1);
        } while(rival == -1);

        do {
            sekundant = SearchForSecundant(&debug, &tid, &size, &LampClock, &msg[2], &Sekundanci, SekundanciBuffer);
            fflush(stdout);
            sleep(1);
        } while(!sekundant);

        do {
            score = Fight(&debug, &tid, &size, &LampClock, &msg[2], &rival, SekundanciBuffer);
            fflush(stdout);
            sleep(1);
        } while(score == -1);
        if (score) {
            continue;
        }
        
        do {
            heal = HealMe(&debug, &tid, &size, &LampClock, &msg[2], &SaleSzpitalne, SaleSzpitalneBuffer);
            fflush(stdout);
            sleep(1);
        } while (!heal);
    }

	MPI_Finalize();
}