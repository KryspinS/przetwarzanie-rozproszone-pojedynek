#include "main.h"
#include "watek_glowny.h"
#include "watek_komunikacyjny.h"

int rank, size, lampClock, priorytet,
    sekundanci, saleSzpitalne,
    aggrementSum, rival, bufer[];
list_t rivalsList[];
state_t stan=InRun;
state_f stan_od=Rival;
pthread_t threadKom, threadMon;
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stateForMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lampMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t priorityMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t aggrementSumMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t buferMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rivalsMut = PTHREAD_MUTEX_INITIALIZER;

void finalizuj()
{
    pthread_mutex_destroy( &stateMut);
    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n" );
    pthread_join(threadKom,NULL);
    free(bufer);
    free(rivals);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}

void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
	    fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
	    MPI_Finalize();
	    exit(-1);
	    break;
        case MPI_THREAD_FUNNELED: 
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	    break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	    break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
	    break;
        default: printf("Nikt nic nie wie\n");
    }
}

int main(int argc, char **argv)
{
    MPI_Status status;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);
    inicjuj_typ_pakietu(); // tworzy typ pakietu
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    pthread_create( &threadKom, NULL, startKomWatek , 0);
    *bufer = malloc(size * sizeof(int));
    list_t *rivals = (list_t *)calloc(size, sizeof(list_t));
    scanf("Podaj ilość sekundantów [min 1]: %d", &sekundanci);  
    scanf("Podaj ilość sal szpitalnych [min (n/2)+1]:%d", &saleSzpitalne);  
    
    mainLoop();
    
    finalizuj();
    return 0;
}

