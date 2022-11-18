// KOD NIE ZAPAMIETUJE KTO POBRAL ZASOB PRZEZ CO DOCHODZI DO BLEDNEGO PRZYDZIELENIA
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define ROOT 0
#define MSG_TAG 100
#define maxi(a,b) (((a)>(b))?(a):(b))

int SearchForRival(int *tid, int *size, int *LampClock, int *msg) {
    MPI_Status status;
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
        printf("[DEBUG][SearchForRival] [%d][%d] Dane ODEBRANE: TID:[%d] LAMP IN:[%d] MSG:[%d] \n", *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1]);
        *LampClock = maxi(*LampClock, msg[0]) + 1;
        
        if (*LampClock > msg[0] && msg[1] == 0) {
            MPI_Send(msg, 3, MPI_INT, status.MPI_SOURCE, MSG_TAG, MPI_COMM_WORLD );
            MPI_Recv(msg, 3, MPI_INT, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (msg[1] == 0) {
                printf("[%d] Wezwany na pojedynek z %d\n", *tid, status.MPI_SOURCE);
                return status.MPI_SOURCE;
            }
        }
    }
    
    printf("[%d] Nie znalazem rywala, szukam dalej!\n", *tid);
    return -1;
}

int SearchForSecundant(int *tid, int *size, int *LampClock, int *msg, int *Sekundanci) {
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
        printf("[DEBUG][SearchForSecundant] [%d][%d] Dane ODEBRANE: TID:[%d] LAMP IN:[%d] MSG:[%d] \n", *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1]);
        
        if (msg[1] != 1 && msg[1] != 2) akc++;
        else if(msg[0] < *LampClock && msg[1] == 1) akc++;
        *LampClock = maxi(*LampClock, msg[0]) + 1;
    }
    int sum = *Sekundanci - (*size - 1 - akc);
    printf("[DEBUG][SearchForSecundant] [%d][%d] Dane ODEBRANE: SUM:[%d] \n", *tid, *LampClock, sum);
    if (sum > 0) {
        return 1;
    }
    
    printf("[%d] Nie znalazem Sekundanata, probuje jeszcze raz!\n", *tid);            
    return 0;
}

int Fight(int *tid, int *size, int *LampClock, int *msg, int *rival) {
    struct timespec spec;
    MPI_Status status;
    int akc,res,shot;
    msg[1] = 2; //2 = walczy
    printf("[%d] Walcze z %d\n", *tid, *rival); 
    clock_gettime(CLOCK_REALTIME, &spec);
    srand(spec.tv_nsec);
    shot = rand()%1000 + 1;

    msg[0] = ++*LampClock;
    msg[2] = shot; 
    MPI_Send(msg, 3, MPI_INT, *rival, MSG_TAG, MPI_COMM_WORLD );
    MPI_Recv(msg, 3, MPI_INT, *rival, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    printf("[DEBUG][Fight] [%d][%d] Dane ODEBRANE: TID:[%d] LAMP IN:[%d] MSG:[%d] SHOT[%d] \n", *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);
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

    *LampClock = maxi(*LampClock, msg[0]) + 1;
    return res;

}

int HealMe(int *tid, int *size, int *LampClock, int *msg, int *SaleSzpitalne) {
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
        printf("[DEBUG][HealMe] [%d][%d] Dane ODEBRANE: TID:[%d] LAMP IN:[%d] MSG:[%d] \n", *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1]);
        
        if (msg[1] != 3) akc++;
        else if(msg[0] < *LampClock && msg[1] == 3) akc++;
        *LampClock = maxi(*LampClock, msg[0]) + 1;
    }
    int sum = *SaleSzpitalne - (*size - 1 - akc);
    printf("[DEBUG][HealMe] [%d][%d] Dane ODEBRANE: SUM:[%d] \n", *tid, *LampClock, sum);
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
	int tid, size,msg[3],
        Sekundanci, SaleSzpitalne, LampClock,
        rival, score, sekundant, heal;
    Sekundanci = atoi(argv[1]);
    SaleSzpitalne = atoi(argv[2]);
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
	printf("Witam jestem zawodnik nr %d!\n", tid);
    while(1) {
        do {
            rival = SearchForRival(&tid, &size, &LampClock, &msg[2]);
        } while(rival == -1);
        fflush(stdout);
        sleep(1);

        do {
            sekundant = SearchForSecundant(&tid, &size, &LampClock, &msg[2], &Sekundanci);
        } while(!sekundant);
        fflush(stdout);
        sleep(1);

        do {
            score = Fight(&tid, &size, &LampClock, &msg[2], &rival);
        } while(score == -1);
        fflush(stdout);
        sleep(1);
        
        do {
            heal = HealMe(&tid, &size, &LampClock, &msg[2], &SaleSzpitalne );
        } while (!heal);
        fflush(stdout);
        sleep(1);
    }

	MPI_Finalize();
}