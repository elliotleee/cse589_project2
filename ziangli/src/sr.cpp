#include "../include/simulator.h"
#include <numeric>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
using namespace std;
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
int seqnum;
int acknum;

int b_nextseqnum = 0;

float RTT = 15;

int WINDOWSIZE = 1;

vector<pkt> Waiting_pktlist;

vector<pkt> Arrive_pktlist;

struct pkt_tim : public pkt{
  public:
  float call_time;
};
vector<pkt_tim> pkttimlist;

float getnexttick(){
    vector<pkt_tim>::iterator pok;
    float sim_time = get_sim_time();
    float mintick = pkttimlist.begin()->call_time;
    for (pok = pkttimlist.begin(); pok != pkttimlist.end(); ++pok)
    {
        if (pok->call_time < mintick)
        {
            mintick = pok->call_time;
        }
    }
  return mintick - sim_time;
}
void updatetimer(){
  if(pkttimlist.size() == 0){
    return;
  }
    float nexttick = getnexttick();
    starttimer(0,nexttick);
}
static pkt_tim* poktotimPacket(struct pkt* pok){	
	  struct pkt_tim* pkt_time = new pkt_tim();
	  pkt_time->acknum = pok->acknum;
	  pkt_time->seqnum = pok->seqnum;
	  strncpy(pkt_time->payload, pok->payload, 20);
	  pkt_time->checksum = pok->checksum;
    pkt_time->call_time = get_sim_time() + RTT;
    return pkt_time;
}
static pkt_tim* inSendingPacket(int seqnum) {
  for (unsigned int i = 0; i < pkttimlist.size(); ++i) {
    pkt_tim* pok = &pkttimlist[i];
    if (pok->seqnum == seqnum) {
      return pok;
    }
  }
  return NULL;
}
int checkSum(struct pkt* packet){
  int s = 0;
  for (int i = 0; i < 20;i++) {
      s += packet->payload[i];
  }
  s += packet->acknum;
  s += packet->seqnum;
  return s;
}
int checkACKSum(struct pkt* packet){
  int s = 0;
  s += packet->acknum;
  s += packet->seqnum;
  return s;
}
void rmSendingPkt(int seqnum) {
    vector<pkt_tim>::iterator pok;
    for (pok = pkttimlist.begin(); pok != pkttimlist.end(); ++pok)
    {
        if (seqnum == pok->seqnum)
        {
            pkttimlist.erase(pok); 
            return;  
        }
    }
}
static pkt* timtoPacket(struct pkt_tim* pkt_tim){	
	  struct pkt* packet = new pkt();
	  packet->acknum = pkt_tim->acknum;
	  packet->seqnum = pkt_tim->seqnum;
	  strncpy(packet->payload, pkt_tim->payload, 20);
	  packet->checksum = pkt_tim->checksum;
    return packet;
}
void sendpkt(struct pkt* pok){
	tolayer3(0,*pok);
}

void send_next_pkt(){
	struct pkt pok = Waiting_pktlist.back();
  struct pkt_tim* pok_tim = poktotimPacket(&pok);
  if (pkttimlist.size() == 0){
    pkttimlist.push_back(*pok_tim);
    printf("[A - send1] %d,%d,%.20s\n", pok.seqnum, pok.acknum, pok.payload);
    sendpkt(&pok);
    starttimer(0,RTT);
  }else{
    pkttimlist.push_back(*pok_tim);
    sendpkt(&pok);
    printf("[A - send2] %d,%d,%.20s\n", pok.seqnum, pok.acknum, pok.payload);
  }
	Waiting_pktlist.pop_back();
} 
static pkt* newPacket(int seqnum,int acknum, char* data){	
	  struct pkt* packet = new pkt();
	  packet->acknum = acknum;
	  packet->seqnum = seqnum;
	  strncpy(packet->payload, data, 20);
	  packet->checksum = checkSum(packet);
    return packet;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message){
   Waiting_pktlist.insert(Waiting_pktlist.begin(),*newPacket(seqnum,acknum,message.data));
   printf("[A - in] %d,%d,%.20s\n", Waiting_pktlist.begin()->seqnum, Waiting_pktlist.begin()->acknum, Waiting_pktlist.begin()->payload);
   seqnum = seqnum + 1;
   if(pkttimlist.size() < WINDOWSIZE && Waiting_pktlist.size() >0){
   		send_next_pkt();
   }  
}
/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet){
	struct pkt_tim* pkt_tim = inSendingPacket(packet.seqnum);
	if(packet.checksum == checkACKSum(&packet) && pkt_tim){
    stoptimer(0);
    rmSendingPkt(packet.seqnum);
    updatetimer();
		if(pkttimlist.size() < WINDOWSIZE && Waiting_pktlist.size() >0){
   			send_next_pkt();
   		}
	}
}
/* called when A's timer goes off */
void A_timerinterrupt(){
    vector<pkt_tim>::iterator pok;
    int minsqe=pkttimlist.begin()->seqnum;
    float mintick = pkttimlist.begin()->call_time;
    for (pok = pkttimlist.begin(); pok != pkttimlist.end(); ++pok)
    {
        if (pok->call_time < mintick)
        {
            mintick = pok->call_time;
            minsqe = pok->seqnum;
        }
    }
    struct pkt_tim* pkt_tim = inSendingPacket(minsqe);
    pkt_tim->call_time = get_sim_time() + RTT;
    sendpkt(pkt_tim);
    updatetimer();
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(){
  seqnum = 0;
  acknum = 1;
  Waiting_pktlist.clear();
  WINDOWSIZE= getwinsize();
  pkttimlist.clear();
}
static pkt* inArrivePacket(int seqnum) {
  for (unsigned int i = 0; i < Arrive_pktlist.size(); ++i) {
    pkt* pok = &Arrive_pktlist[i];
    if (pok->seqnum == seqnum) {
      return pok;
    }
  }
  return NULL;
}
void rmArrivePkt(int seqnum) {
    vector<pkt>::iterator pok;
    for (pok = Arrive_pktlist.begin(); pok != Arrive_pktlist.end(); ++pok)
    {
        if (seqnum == pok->seqnum)
        {
            Arrive_pktlist.erase(pok); 
            return;  
        }
    }
}
/* Note that with simplex transfer from a-to-B, there is no B_output() */
static pkt* makeACKPacket( int seqnum, int acknum){	
	  struct pkt* packet = new pkt();
	  packet->seqnum = seqnum;
	  packet->acknum = acknum;
	  memset(packet->payload,0,sizeof(packet->payload));
	  packet->checksum = seqnum + acknum;
    return packet;
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet){
  if (packet.checksum != checkSum(&packet)) {
    return;
  }
  printf("[B - ACK1] %d,%d,%.20s\n",  packet.seqnum, packet.acknum, packet.payload);
  if(inArrivePacket(packet.seqnum) || packet.seqnum < b_nextseqnum){
      struct pkt *ack = makeACKPacket(packet.seqnum, 1);
      tolayer3(1, *ack);
      return;
  }
  struct pkt *ack = makeACKPacket(packet.seqnum, 1);
  tolayer3(1, *ack);
  Arrive_pktlist.push_back(packet);

  while(!Arrive_pktlist.empty()){
    vector<pkt>::iterator pok;
    int minseq = Arrive_pktlist.begin()->seqnum;
    for (pok = Arrive_pktlist.begin(); pok != Arrive_pktlist.end(); ++pok)
    {
        if (pok->seqnum < minseq)
        {
            minseq = pok->seqnum;
        }
    }
      if (minseq == b_nextseqnum){
          struct pkt* packeta = inArrivePacket(minseq);
          tolayer5(1, packeta->payload);
          rmArrivePkt(minseq);
          b_nextseqnum += 1;
        }else{
          break;
        }
  }
}


/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(){
    b_nextseqnum = 0;
    Arrive_pktlist.clear();
}
