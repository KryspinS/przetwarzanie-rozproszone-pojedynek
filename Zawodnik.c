#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define REQ 0
#define ACK 1
#define NACK -1
#define RIVAL 0
#define SEKUNDANT 1
#define FIGHT 2
#define HEAL 3

#define MSG_TAG 100
#define maxi(a, b) (((a) > (b)) ? (a) : (b))

int CheckRecives(int *debug, int *tid, int *size, int *LampClock,
                 int *msg, int stocType, int *buf, int *reqTo, int *reqList)
{
    MPI_Status status;
    int ack = 0, count = 0;
    do
    {
        MPI_Recv(msg, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        *LampClock = maxi(*LampClock, msg[0]) + 1;
        if (*debug)
            printf("[DEBUG][RECEIVE][%d] [%d]:[%d] Dane: OD:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", stocType, *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);

        if (stocType == msg[1]) // MSG dotyczy tego samego zasobu
        {
            count++;

            if (msg[1] == FIGHT) // Jesli typ Fight to zakoncz petle - mozesz odebrac tylko jesli walczysz
            {
                return 1;
            }
            else if (msg[2] == REQ) 
            {
                if (msg[1] == RIVAL && status.MPI_SOURCE == *reqTo) // Jesli tan sam ktorego pytalismy prosi o pojedynek zapisz rywala
                {
                    return *reqTo;
                }
                else if (msg[1] == RIVAL && status.MPI_SOURCE != *reqTo) // Jesli ktos inny prosi o pojedynek, a juz czekamy na czyjas decyzje zapisz w buforze i czekaj
                {
                    msg[0] = ++*LampClock;
                    msg[2] = NACK;
                    MPI_Send(msg, 3, MPI_INT, status.MPI_SOURCE, MSG_TAG, MPI_COMM_WORLD);
                    if (*debug) printf("[DEBUG][SEND][%d] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", stocType, *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);
                }

                else if (msg[0] < reqList[status.MPI_SOURCE] || (*tid > status.MPI_SOURCE && msg[0] == reqList[status.MPI_SOURCE]))
                {
                    ack++;
                }
                else
                {
                    buf[status.MPI_SOURCE] = 1;
                }
            }
            else if (msg[2] == ACK) // ACK
            {
                if (msg[1] == RIVAL) // Otrzymano akceptacje pojedynku
                {
                    return status.MPI_SOURCE;
                }
                ack++;
            }
            else // N-ACK
            {
                for (int i = 0; i < *size; i++) // Jesli kros prosil o pojedynek to wyslij akceptacje i zapisz rywala
                {
                    if (i == *tid || buf[i] == 0){continue;}

                    msg[0] = ++*LampClock;
                    msg[2] = ACK;
                    MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
                    if (*debug) printf("[DEBUG][SEND][%d] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", stocType, *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);
                    return i;
                }
                return -1;
            }
        }
        else // MSG dotyczy innego zasobu to wyslka ACK
        {
            if (msg[2] == REQ)
            {
                msg[0] = ++*LampClock;
                msg[2] = (msg[1] == RIVAL) ? NACK : ACK;
                MPI_Send(msg, 3, MPI_INT, status.MPI_SOURCE, MSG_TAG, MPI_COMM_WORLD);
                if (*debug) printf("[DEBUG][SEND][%d] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", stocType, *tid, *LampClock, status.MPI_SOURCE, msg[0], msg[1], msg[2]);
            }
        }
    } while (count < *size - 1);
    return ack;
}

int CommunicationController(int *debug, int *tid, int *size, int *LampClock,
                            int *msg, int stocType, int *buf)
{
    MPI_Status status;
    int *reqList = (int *)calloc(*size, sizeof(int));
    int ack=0;

    for (int i = 0; i < *size; i++)
    {
        if (i == *tid){continue;}

        msg[0] = ++*LampClock;
        msg[1] = stocType;
        msg[2] = REQ;
        MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
        if (*debug) printf("[DEBUG][SEND][%d] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", stocType, *tid, *LampClock, i, msg[0], msg[1], msg[2]);
        reqList[i] = *LampClock;

        if (stocType == RIVAL)
        {
            ack = CheckRecives(debug, tid, size, LampClock, msg, stocType, buf, &i, reqList);
            if (ack != -1) return ack;
        }
    }
    if (ack == -1 ) {return ack;}

    if (*debug) printf("[DEBUG][CommunicationController][%d] [%d]:[%d] Dane: REQ Priority: 0:%d 1:%d 2:%d 3:%d \n", stocType, *tid, *LampClock, reqList[0], reqList[1], reqList[2], reqList[3]);
    ack = CheckRecives(debug, tid, size, LampClock, msg, stocType, buf, tid, reqList);
    return ack;
}

int SearchForRival(int *debug, int *tid, int *size, int *LampClock, int *msg, int *buf)
{
    printf("[%d]:[%d] Szukam przeciwnika\n", *tid, *LampClock);

    int rival = CommunicationController(debug, tid, size, LampClock, msg, RIVAL, buf);
    if (rival != -1) printf("[%d]:[%d] Moim przeciwnikiem jest %d\n", *tid, *LampClock, rival);

    for (int i = 0; i < *size; i++) // Odsylanie niedostepnosci os. oczekujacym
    {
        if (i == *tid || buf[i] == 0){continue;}

        msg[0] = ++*LampClock;
        msg[1] = RIVAL;
        msg[2] = NACK;
        MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
        if (*debug) printf("[DEBUG][SEND][%d] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", msg[1], *tid, *LampClock, i, msg[0], msg[1], msg[2]);
    }

    if (rival == -1) printf("[%d]:[%d] Nie znalazem rywala, szukam dalej!\n", *tid, *LampClock);
    return rival;
}

int SearchForSecundant(int *debug, int *tid, int *size, int *LampClock,
                       int *msg, int *Sekundanci, int *buf)
{
    MPI_Status status;
    printf("[%d]:[%d] Szukam Sekundanta\n", *tid, *LampClock);

    int ack = CommunicationController(debug, tid, size, LampClock, msg, SEKUNDANT, buf);

    int sum = *Sekundanci - (*size - 1 - ack);
    if (*debug)
        printf("[DEBUG][SearchForSecundant] [%d]:[%d] Dane ODEBRANE: SUM:[%d] \n", *tid, *LampClock, sum);
    if (sum > 0)
    {
        return 1;
    }

    printf("[%d]:[%d] Nie znalazem Sekundanata, probuje jeszcze raz!\n", *tid, *LampClock);
    return 0;
}

int Fight(int *debug, int *tid, int *size, int *LampClock, int *msg, int *rival, int *buf)
{
    struct timespec spec;
    MPI_Status status;
    int shot;
    int *reqList = (int *)calloc(*size, sizeof(int));
    printf("[%d]:[%d] Walcze z %d\n", *tid, *LampClock, *rival);
    clock_gettime(CLOCK_REALTIME, &spec);
    srand(spec.tv_nsec);
    shot = rand() % 1000 + 1; // ten konflikt zasluguje na losowosc dlatego nie Lamport

    msg[0] = ++*LampClock;
    msg[1] = FIGHT;
    msg[2] = shot;

    MPI_Send(msg, 3, MPI_INT, *rival, MSG_TAG, MPI_COMM_WORLD);
    if (*debug) printf("[DEBUG][SEND][2] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", *tid, *LampClock, *rival, msg[0], msg[1], msg[2]);
    reqList[*rival] = *LampClock;
    int v = CheckRecives(debug, tid, size, LampClock, msg, FIGHT, buf, rival, reqList);

    if (msg[2] > shot)
    {
        printf("[%d]:[%d] Poleglem z %d\n", *tid, *LampClock, *rival);
        return 0;
    }
    else if (msg[2] < shot)
    {
        printf("[%d]:[%d] Wygralem z %d\n", *tid, *LampClock, *rival);
        return 1;
    }
    else 
    {
        printf("[%d]:[%d] Kule trafiły w siebie, powtarzam z %d!\n", *tid, *LampClock, *rival);
        return -1;
    }


    for (int i = 0; i < *size; i++)
    {
        if (i == *tid || buf[i] == 0){continue;}
            buf[i] = 0;
            msg[0] = ++*LampClock;
            msg[1] = SEKUNDANT;
            msg[2] = ACK;
            MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
            if (*debug) printf("[DEBUG][SEND][2] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", *tid, *LampClock, i, msg[0], msg[1], msg[2]);
    }
}

int HealMe(int *debug, int *tid, int *size, int *LampClock, int *msg, int *SaleSzpitalne, int *buf)
{
    struct timespec spec;
    MPI_Status status;
    printf("[%d]:[%d] Szukam wolnego lozka\n", *tid, *LampClock);

    int ack = CommunicationController(debug, tid, size, LampClock, msg, HEAL, buf);
    int sum = *SaleSzpitalne - (*size - 1 - ack);

    if (sum > 0)
    {
        clock_gettime(CLOCK_REALTIME, &spec);
        srand(spec.tv_nsec);
        int nurseSpeed = rand() % 3 + 1;

        printf("[%d]:[%d] Znalazlem lozko, bede leczyl sie %ds\n", *tid, *LampClock, nurseSpeed);
        fflush(stdout);
        
        sleep(nurseSpeed);

        printf("[%d]:[%d] Wyleczony\n", *tid, *LampClock);
        for (int i = 0; i < *size; i++)
        {
            if (i == *tid || buf[i] == 0){continue;}
            buf[i] = 0;
            msg[0] = ++*LampClock;
            msg[1] = HEAL;
            msg[2] = ACK;
            MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
            if (*debug) printf("[DEBUG][SEND][3] [%d]:[%d] Dane: DO:[%d] LAMP:[%d] TYP:[%d] MSG:[%d] \n", *tid, *LampClock, i, msg[0], msg[1], msg[2]);
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
    Sekundanci = atoi(argv[1]);
    SaleSzpitalne = atoi(argv[2]);
    debug = atoi(argv[3]);
    LampClock = 0;


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &tid);
    if (Sekundanci*2 <= size)
    {
        printf("[%d]:[%d] Sekundantow musi byc minimum (n/2)+1, a podano: %d\n", tid, LampClock, Sekundanci);
        MPI_Finalize();
        exit(0);
    }
    if (SaleSzpitalne < 1)
    {
        printf("[%d]:[%d] Sal szpitalnych musi byc conajmniej jedna, a podano: %d\n", tid, LampClock, SaleSzpitalne);
        MPI_Finalize();
        exit(0);
    }

    int *SekundanciBuffer = (int *)calloc(size, sizeof(int));
    int *SaleSzpitalneBuffer = (int *)calloc(size, sizeof(int));
    int *PojedybekBuffer = (int *)calloc(size, sizeof(int));

    printf("[%d]:[%d] Witam!\n", tid, LampClock);
    while (1)
    {
        do
        {
            rival = SearchForRival(&debug, &tid, &size, &LampClock, &msg[3], PojedybekBuffer);
            fflush(stdout);
            sleep(1);
        } while (rival == -1);

        do
        {
            sekundant = SearchForSecundant(&debug, &tid, &size, &LampClock, &msg[3], &Sekundanci, SekundanciBuffer);
            fflush(stdout);
            sleep(1);
        } while (!sekundant);

        do
        {
            score = Fight(&debug, &tid, &size, &LampClock, &msg[3], &rival, SekundanciBuffer);
            fflush(stdout);
            sleep(1);
        } while (score == -1);
        if (score)
        {
            continue;
        }

        do
        {
            heal = HealMe(&debug, &tid, &size, &LampClock, &msg[3], &SaleSzpitalne, SaleSzpitalneBuffer);
            fflush(stdout);
            sleep(1);
        } while (!heal);
    }

    MPI_Finalize();
}