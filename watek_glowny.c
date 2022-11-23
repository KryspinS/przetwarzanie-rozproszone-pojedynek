#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    int tag;

    while (1) {
        SearchForRival();
		SearchForSecundant();
        Fight();
        if (stan == ToHeal) {HealMe();}
    }
}

void SearchForRival()
{
    packet_t *pkt = malloc(sizeof(packet_t));

    //TODO lista min max i wybór rywala

}

void SearchForSecundant()
{
    packet_t *pkt = malloc(sizeof(packet_t));
    pkt->ts = lampClock;
    pkt->data = SEKUNDANT;
    aggrementSum = 0;

    changeState(InSecundantSend);
    for(int i=0; i<size; i++)
    {
		sendPacket( pkt, i, REQ);
    }
    changeState(InSecundantMonitor);

    while(stan!=InFree)
    {
        if (aggrementSum>0) {changeState(InFree);}
        sleep(SEC_IN_STATE);
    }
}

void Fight() 
{
    srandom(rank);
    int shot = rand() % 1000 + 1;
    packet_t *pkt = malloc(sizeof(packet_t));
    pkt->data = FIGHT;
    pkt->value = shot;
    pkt->ts = lampClock;

    changeState(InFightSend);
    sendPacket( pkt, rival, REQ);
    changeState(InFightMonitor);
    
    while(stan!=InFree)
    {
        if (aggrementSum>0) {changeState(InFree);}
        sleep(SEC_IN_STATE);
    }
    changeState(InFree);

    for(int i=0; i<size; i++)
    {
        pkt->data = SEKUNDANT;
        sendPacket( pkt, i, FREE);
    }
    
    
    if (aggrementSum>shot) 
    {
        changeState(ToHeal);
        println("Zostałem pokonany")
    }
    
   
}

void HealMe()
{
    srandom(rank);
    int nurseSpeed = rand() % 3 + 1;
    packet_t *pkt = malloc(sizeof(packet_t));
    pkt->data = HEAL;
    pkt->ts = lampClock;
    aggrementSum = 0;

    changeState(InHealSend);
    for(int i=0; i<size; i++)
    {
		sendPacket( pkt, i, REQ);
    }
    changeState(InHealMonitor);

    while(stan!=InFree)
    {
        if (aggrementSum>0) {
            changeState(InRun);
            sleep(nurseSpeed);
            changeState(InFree);
        }
        sleep(SEC_IN_STATE);
    }

    for(int i=0; i<size; i++)
    {
        pkt->data = HEAL;
		sendPacket( pkt, i, FREE);
    }
}