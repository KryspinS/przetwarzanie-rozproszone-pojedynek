//DOSTEP DO SEKCJI NIE JEST PRAWIDLOWO SPRAWDZANY
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define ROOT 0
#define MSG_TAG 100
#define maxi(a,b) (((a)>(b))?(a):(b))

int SearchForRival(int *debug, int *tid, int *size, int *LampClock, int *msg) {
    MPI_Status status;
    int rival = -1;
    msg[1] = 0; //0 = Szuka rywala
    printf("[%d] Szukam przeciwnika\n", *tid);

    for(int i=0; i<*size; i++){
        if (i == *tid) {continue;}
        msg[0] = ++*LampClock;
        MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD );
    }

    for(int i=0; i<*size; i++){
        if (i == *tid) {continue;}
        MPI_Recv(msg, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(*debug) printf("[DEBUG][SearchForRival] [%d][%d] Dane ODEBRANE: TID:[%d] LAMP IN:[%d] MSG:[%d] \n", *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1]);
        *LampClock = maxi(*LampClock, msg[0]) + 1;
        
        if (*LampClock > msg[0] && msg[1] == 0 && rival == -1) {
            MPI_Send(msg, 3, MPI_INT, status.MPI_SOURCE, MSG_TAG, MPI_COMM_WORLD );
            MPI_Recv(msg, 3, MPI_INT, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (msg[1] == 0) {
                printf("[%d] Wezwany na pojedynek z %d\n", *tid, status.MPI_SOURCE);
                rival = status.MPI_SOURCE;
            }
        }
    }
    if(rival != -1) {
        return rival;
    }
    
    printf("[%d] Nie znalazem rywala, szukam dalej!\n", *tid);
    return -1;
}

int SearchForSecundant(int *debug, int *tid, int *size, int *LampClock, int *msg, int *Sekundanci, int *buf) {
    MPI_Status status;
    int akc;
    msg[1] = 1; //1 = Szuka Sekundanta
    printf("[%d] Szukam Sekundanta\n", *tid);    

    for(int i=0; i<*size; i++){
        if (i == *tid) {continue;}
        msg[0] = ++*LampClock;
        MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD );
    }
    akc=0;

    for(int i=0; i<*size; i++){
        if (i == *tid) {continue;}
        MPI_Recv(msg, 3, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(*debug) printf("[DEBUG][SearchForSecundant] [%d][%d] Dane ODEBRANE: TID:[%d] LAMP IN:[%d] MSG:[%d] \n", *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1]);
        
        
        if (msg[1] != 1 && msg[1] != 2) {
            akc++;
            buf[status.MPI_SOURCE] = 0;
        }
        else if(msg[0] < *LampClock && msg[1] == 1) {
            if(!buf[status.MPI_SOURCE]){
                akc++;
            } 
        }
        else {
            buf[status.MPI_SOURCE] = 1;
        }

        if(*debug) printf("[DEBUG][SearchForSecundant] [%d][%d] BUFFER: [%d] [%d] [%d] [%d] \n", *tid, *LampClock, buf[0], buf[1], buf[2], buf[3]);
        *LampClock = maxi(*LampClock, msg[0]) + 1;
    }
    int sum = *Sekundanci - (*size - 1 - akc);
    if(*debug) printf("[DEBUG][SearchForSecundant] [%d][%d] Dane ODEBRANE: SUM:[%d] \n", *tid, *LampClock, sum);
    if (sum > 0) {
        return 1;
    }
    
    printf("[%d] Nie znalazem Sekundanata, probuje jeszcze raz!\n", *tid);            
    return 0;
}

int Fight(int *debug, int *tid, int *size, int *LampClock, int *msg, int *rival) {
    struct timespec spec;
    MPI_Status status;
    int akc,res,shot, rivShot, rivMsg;
    msg[1] = 2; //2 = walczy
    printf("[%d] Walcze z %d\n", *tid, *rival); 
    clock_gettime(CLOCK_REALTIME, &spec);
    srand(spec.tv_nsec);
    shot = rand()%1000 + 1;

    msg[2] = shot; 

    for(int i=0; i<*size; i++){
        if (i == *tid) {continue;}
        msg[0] = ++*LampClock;
        MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD );
    }

    for(int i=0; i<*size; i++){
        if (i == *tid) {continue;}
        MPI_Recv(msg, 3, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_SOURCE == *rival) {
                rivShot = msg[2];
                rivMsg = msg[1];
        }
        *LampClock = maxi(*LampClock, msg[0]) + 1;
    }
    
    
    if(*debug) printf("[DEBUG][Fight] [%d][%d] Dane ODEBRANE: TID:[%d] LAMP IN:[%d] MSG:[%d] SHOT[%d] \n", *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);
    if (msg[2] < shot && msg[1] == 2) {
        printf("[%d] Wygralem z %d\n", *tid, *rival);
        res = 1;    
    } else if (msg[2] > shot && msg[1] == 2) {
        printf("[%d] Poleglem z %d\n", *tid, *rival);    
        res = 0;    
    } else {
        printf("[%d] Walcze nadal z %d!\n", *tid, *rival);            
        res = -1;
    }

    return res;

}

int HealMe(int *debug, int *tid, int *size, int *LampClock, int *msg, int *SaleSzpitalne, int *buf) {
    struct timespec spec;
    MPI_Status status;
    int akc, nurseSpeed;
    msg[1] = 3; //3 = Szuka Sali szpitalnej
    printf("[%d] Szukam wolnego lozka\n", *tid);    

    for(int i=0; i<*size; i++){
        if (i == *tid) {continue;}
        msg[0] = ++*LampClock;
        MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD );
    }
    akc=0;

    for(int i=0; i<*size; i++){
        if (i == *tid) {continue;}
        MPI_Recv(msg, 3, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(*debug) printf("[DEBUG][HealMe] [%d][%d] Dane ODEBRANE: TID:[%d] LAMP IN:[%d] MSG:[%d] \n", *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1]);
        
        if (msg[1] != 3) {
            akc++;
            buf[status.MPI_SOURCE] = 0;
        }
        else if(msg[0] > *LampClock && msg[1] == 3){
            if(!buf[status.MPI_SOURCE]) {
                akc++;
            }
        }
        else {
            buf[status.MPI_SOURCE] = 1;
        }

        if(*debug) printf("[DEBUG][HealMe] [%d][%d] BUFFER: [%d] [%d] [%d] [%d] \n", *tid, *LampClock, buf[0], buf[1], buf[2], buf[3]);
        *LampClock = maxi(*LampClock, msg[0]) + 1;

    }
    int sum = *SaleSzpitalne - (*size - 1 - akc);
    
    if(*debug) printf("[DEBUG][HealMe] [%d][%d] Dane ODEBRANE: SUM:[%d] \n", *tid, *LampClock, sum);
    
    if (sum > 0) {
        clock_gettime(CLOCK_REALTIME, &spec);
        srand(spec.tv_nsec);
        nurseSpeed = rand()%10 + 1;
        
        printf("[%d] Znalazlem lozko, bede leczyl sie %ds\n", *tid, nurseSpeed);    
        sleep(nurseSpeed);
        
        printf("[%d] Wyleczony\n", *tid);    
        return 1;
    }
    
    printf("[%d] Nadal szukam lozka w szpitalu!\n", *tid);   
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
        printf("[%d] Sekundantow musi byc conajmniej dwoch, a podano: %d\n", tid, Sekundanci);   
        exit(0); 
    }
    if(SaleSzpitalne < 1) {
        printf("[%d] Sal szpitalnych musi byc conajmniej jedna, a podano: %d\n", tid, SaleSzpitalne);    
        exit(0); 
    }

	MPI_Init(&argc, &argv);

	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Comm_rank( MPI_COMM_WORLD, &tid );

    SekundanciBuffer = (int*) calloc(size, sizeof(int));
    SaleSzpitalneBuffer = (int*) calloc(size, sizeof(int));


	printf("Witam jestem zawodnik nr %d!\n", tid);
	// if(*debug) printf("[DEBUG] [%d] SEK: [%d] [%d] [%d] [%d], SSZ: [%d] [%d] [%d] [%d]!\n", tid, SekundanciBuffer[0], SekundanciBuffer[1], SekundanciBuffer[2], SekundanciBuffer[3], SaleSzpitalneBuffer[0], SaleSzpitalneBuffer[1], SaleSzpitalneBuffer[2], SaleSzpitalneBuffer[3]);
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
            score = Fight(&debug, &tid, &size, &LampClock, &msg[2], &rival);
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