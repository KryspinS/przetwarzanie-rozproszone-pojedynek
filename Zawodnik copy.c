#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define ROOT 0
#define MSG_TAG 100


long getLamportTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (long)ts.tv_nsec;
}

int SearchForRival(int *tid, int *size, MPI_Status *status, long* msg) {
    //TODO: szukaj wolnego gracza
    long buf[*size][3];
    msg[0] = (long) *tid;
    msg[1] = (long) getLamportTime();
    msg[2] = 1;
    printf("[%d] Szukam przeciwnika\n", *tid);
    printf("[DEBUG] [%d] Dane: TID:[%ld] LAMP:[%ld] MSG:[%ld] \n", *tid, msg[0], msg[1], msg[2]);
    for (int i=0; i<*size; i++) {
        // if (*tid == i) {
            MPI_Gather(msg, 3, MPI_LONG, &buf, 3, MPI_LONG, i, MPI_COMM_WORLD);
        // } else {
            // MPI_Gather(msg, 3, MPI_LONG, NULL, 3, MPI_LONG, i, MPI_COMM_WORLD);
        // }
        printf("[DEBUG] [%d][%d] Dane PO: TID:[%ld] LAMP:[%ld] MSG:[%ld] \n", *tid, i, buf[i][0], buf[i][1], buf[i][2]);
    }
    // MPI_Recv(msg, 2, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &*status);
    // printf("[%d] Moim rywalem jest %ld\n", *tid, msg[0]);
    
    

}

int SearchForSecundant(int *tid, int *size, MPI_Status *status, int *Sekundanci) {
    printf("[%d] Szukam Sekundanta\n", *tid);    
    //TODO: szukaj wolnego sekundanta
}

int Fight(int *tid, int *size, MPI_Status *status) {
    printf("[%d] Walcze\n", *tid);    
    // TODO: wyzmacz zwyciezce
    printf("[%d] Poleglem\n", *tid);    
    printf("[%d] Zwyciezylem\n", *tid);
}

int HealMe(int *tid, int *size, MPI_Status *status, int *SaleSzpitalne) {
    //TODO: sprawdz dostep do zasobu
    printf("[%d] Lecze sie\n", *tid);    


    //TODO: Leczenie
    printf("[%d] Wyleczony\n", *tid);    
}


int main(int argc, char **argv)
{
	int tid, 
        size, 
        Sekundanci, 
        SaleSzpitalne;
    long msg[3];
	MPI_Status status;
    Sekundanci = atoi(argv[1]);
    SaleSzpitalne = atoi(argv[2]);

	MPI_Init(&argc, &argv);

	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Comm_rank( MPI_COMM_WORLD, &tid );
	printf("Witam jestem zawodnik nr %d!\n", tid);
    msg[0] = (long) tid;
    msg[1] = (long) getLamportTime();
    msg[2] = 5;
    //while(1) {
        SearchForRival(&tid, &size, &status, &msg[3]);
        /* SearchForSecundant(&tid, &size, &status &Sekundanci );
        Fight(&tid, &size, &status );
        HealMe(&tid, &size, &status, &SaleSzpitalne ); */
        sleep(2);
    //}

	MPI_Finalize();
}