#include "../include/simulator.h"
#include <numeric>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

int b_nextseqnum;

float RTT = 22;

static int WINDOWSIZE = 1;

vector<pkt> Waiting_pktlist;

vector<pkt> Sending_pktlist;

pkt* inSendingPacket(int seqnum) {
  for (unsigned int i = 0; i < Sending_pktlist.size(); ++i) {
    pkt* pok = &Sending_pktlist[i];
    if (pok->seqnum == seqnum) {
      return pok;
    }
  }
  return NULL;
}
void rmSendingPkt(int seqnum) {

    vector<pkt>::iterator pok;

    for (pok = Sending_pktlist.begin(); pok != Sending_pktlist.end(); ++pok)
    {
        if (seqnum == pok->seqnum)
        {
            Sending_pktlist.erase(pok); 
            return;  
        }
    }
}
void sendpkt(struct pkt* pok){
	tolayer3(0,*pok);
	starttimer(0,RTT);
}
void send_next_pkt(){
	struct pkt pok = Waiting_pktlist.back();
	Sending_pktlist.push_back(pok);
	sendpkt(&pok);
	Waiting_pktlist.pop_back();
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
   seqnum = seqnum + 1;
   if(Sending_pktlist.size() < WINDOWSIZE && Waiting_pktlist.size() >0){
   		send_next_pkt();
   } 
   
}
/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet){
	struct pkt* pok = inSendingPacket(packet.seqnum);
	if(packet.checksum == checkSum(&packet) && packet.acknum == 1 && pok){
		stoptimer(0);
		rmSendingPkt(packet.seqnum);
		if(Sending_pktlist.size() < WINDOWSIZE && Waiting_pktlist.size() >0){
   			send_next_pkt();
   		}
	}
}
/* called when A's timer goes off */
void A_timerinterrupt(){
		struct pkt pok = Sending_pktlist.front();
		sendpkt(&pok);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(){
  seqnum = 0;
  acknum = 1;
  Sending_pktlist.clear();
  Waiting_pktlist.clear();
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
  if(packet.seqnum != b_nextseqnum){
    struct pkt *ack = makeACKPacket(packet.seqnum, 1);
    tolayer3(1, *ack);
    return;
  }else{
    struct pkt *ack = makeACKPacket(packet.seqnum, 1);
    tolayer3(1, *ack);
    tolayer5(1, packet.payload);

    b_nextseqnum = packet.seqnum + 1;
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(){
    b_nextseqnum = 0;
}
