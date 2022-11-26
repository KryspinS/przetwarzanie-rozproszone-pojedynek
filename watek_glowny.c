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
        Fight();
        if (stan == ToHeal)
        {
            HealMe();
        }
    }
}

void SearchForRival()
{
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

    free(pkt);
}

void SearchForSecundant()
{
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
    free(pkt);
}

void Fight()
{
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
        println("Zostałem pokonany")
    }
    free(pkt);
}

void HealMe()
{
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
            sleep(nurseSpeed);
            changeState(InFree);
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