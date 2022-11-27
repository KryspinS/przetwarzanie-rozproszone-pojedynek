#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    int tag;

    while (1)
    {
        SearchForRival();
        SearchForSecundant();
        FightWithRival();
        if (stan == ToHeal) HealYourSelf();
        else debug("Wygrałem\n");
    }
}

void SearchForRival()
{
    debug("Szukam rywala\n");
    changeStateFor(Rival);
    packet_t *pkt = malloc(sizeof(packet_t));
    setPriority();
    pkt->ts = priorytet;
    pkt->data = Rival;
    increaseAggrementSum(0);
    setBufer(rank, priorytet);

    changeState(InSend);
    for (int i = 0; i < size; i++)
    {
        if(i == rank) continue;
        sendPacket(pkt, i, REQ);
    }
    changeState(InMonitor);

    while (stan != InFree)
    {
        if (aggrementSum == size)
        {
            changeState(InFree);
            sortAndChooseRival();
        }
        sleep(SEC_IN_STATE);
    }

    debug("Będę walczył z %d\n", rival);


    free(pkt);
}

void SearchForSecundant()
{
    debug("Szukam sekundanta\n");
    changeStateFor(Sekundant);
    packet_t *pkt = malloc(sizeof(packet_t));
    setPriority();
    pkt->ts = priorytet;
    pkt->data = Sekundant;
    increaseAggrementSum(0);

    changeState(InSend);
    for (int i = 0; i < size; i++)
    {
        if(i == rank) continue;
        sendPacket(pkt, i, REQ);
    }
    changeState(InMonitor);

    while (stan != InFree)
    {
        if (sekundanci-(size-1-aggrementSum ) > 0)
        {
            changeState(InFree);
        }
        sleep(SEC_IN_STATE);
    }
    debug("Znalazłem sekundanta\n");
    free(pkt);
}

void FightWithRival()
{
    debug("Walczę z %d\n", rival);
    changeStateFor(Fight);
    srandom(rank);
    int shot = rand() % 1000 + 1;
    packet_t *pkt = malloc(sizeof(packet_t));
    setPriority();
    pkt->ts = priorytet;
    pkt->data = Fight;
    pkt->value = shot;
    increaseAggrementSum(0);

    changeState(InSend);
    sendPacket(pkt, rival, REQ);
    changeState(InMonitor);

    while (stan != InFree)
    {
        if (aggrementSum > 0)
        {
            changeState(InFree);
        }
        sleep(SEC_IN_STATE);
    }

    pkt->data = Sekundant;
    for (int i = 0; i < size; i++)
    {
        if(bufer[i] == TRUE)
        {
            setBufer(i, FALSE);
            sendPacket(pkt, i, FREE);
        }
    }

    if (aggrementSum > shot || (aggrementSum == shot && rank < rival))
    {
        changeState(ToHeal);
        debug("Zostałem pokonany\n")
    }
    free(pkt);
}

void HealYourSelf()
{
    debug("Będę się leczył\n")
    changeStateFor(Heal);
    srandom(rank);
    int nurseSpeed = rand() % 3 + 1;
    packet_t *pkt = malloc(sizeof(packet_t));
    setPriority();
    pkt->ts = priorytet;
    pkt->data = Heal;
    increaseAggrementSum(0);

    changeState(InSend);
    for (int i = 0; i < size; i++)
    {
        if(i == rank) continue;
        sendPacket(pkt, i, REQ);
    }
    changeState(InMonitor);

    while (stan != InFree)
    {
        if (saleSzpitalne - (size - 1 - aggrementSum) > 0)
        {
            changeState(InRun);
            debug("Leczę się\n");
            sleep(nurseSpeed);
            changeState(InFree);
            debug("wyleczony\n");
        }
        sleep(SEC_IN_STATE);
    }

    pkt->data = Heal;
    for (int i = 0; i < size; i++)
    {
        if(bufer[i] == TRUE)
        {
            setBufer(i, FALSE);
            sendPacket(pkt, i, FREE);
        }
    }
    free(pkt);
}