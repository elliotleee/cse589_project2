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
int a_nextseqnum = 0;
float RTT = 35.0;

int WINDOWSIZE = 1;
float delay = 1;

vector<pkt> Waiting_pktlist;


struct pkt_tim : public pkt{
  public:
  float call_time;
 };

vector<pkt_tim> pkttimlist;
vector<pkt_tim> ackpkttimlist;


float getnexttick(){
  float sim_time = get_sim_time();
  float mintick = pkttimlist.begin()->call_time;
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
 static pkt_tim* inACKSendingPacket(int seqnum) {
  for (unsigned int i = 0; i < ackpkttimlist.size(); ++i) {
    pkt_tim* pok = &ackpkttimlist[i];
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
    for (pok = pkttimlist.begin(); pok != pkttimlist.end(); ++pok){
        if (seqnum == pok->seqnum){
            pkttimlist.erase(pok); 
            return;  
        }
    }
    }
void rmAckSendingPkt(int seqnum) {
    vector<pkt_tim>::iterator pok;
    for (pok = ackpkttimlist.begin(); pok != ackpkttimlist.end(); ++pok){
        if (seqnum == pok->seqnum){
            ackpkttimlist.erase(pok); 
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
    sendpkt(&pok);
    starttimer(0,RTT);
  }else{
    pkttimlist.push_back(*pok_tim);
    sendpkt(&pok);
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

void A_output(struct msg message){
   Waiting_pktlist.insert(Waiting_pktlist.begin(),*newPacket(seqnum,acknum,message.data));
   seqnum = seqnum + 1;
   if(pkttimlist.size() < WINDOWSIZE && Waiting_pktlist.size() >0){
      send_next_pkt();
   }  
 }
void A_input(struct pkt packet){
  struct pkt_tim* pkt_time = inSendingPacket(packet.seqnum);
  float time = get_sim_time();
  if(packet.checksum == checkACKSum(&packet) && pkt_time){
        if(time - (pkt_time->call_time -RTT) < delay ){
         return;
        }
    if(inACKSendingPacket(packet.seqnum)){
      return;
    }
    ackpkttimlist.push_back(*pkt_time);
    if(packet.seqnum == a_nextseqnum){
      stoptimer(0);
      while(!ackpkttimlist.empty()){
            if(ackpkttimlist.begin()->seqnum == a_nextseqnum){
                rmSendingPkt(ackpkttimlist.begin()->seqnum);
                rmAckSendingPkt(ackpkttimlist.begin()->seqnum);
                a_nextseqnum += 1;
            }else{
              break;
            }
      }
      while(pkttimlist.size() < WINDOWSIZE && Waiting_pktlist.size() >0){
          send_next_pkt();  
      }
      updatetimer();
      return;
    }
  }
 }

void A_timerinterrupt(){
    ackpkttimlist.clear();
    vector<pkt_tim>::iterator poke;
    int i = 0;
    for (poke = pkttimlist.begin(); poke != pkttimlist.end(); ++poke){
        struct pkt_tim* pkt_time = inSendingPacket(poke->seqnum);
        pkt_time->call_time = get_sim_time() + RTT + 2*i;
        i++;
        struct pkt* pkt_2 = timtoPacket(pkt_time);
        sendpkt(pkt_2);
    }
    starttimer(0,RTT);
 }    

void A_init(){
  seqnum = 0;
  acknum = 0;
  Waiting_pktlist.clear();
  WINDOWSIZE= getwinsize();
  pkttimlist.clear();
  a_nextseqnum = 0;
 }

static pkt* makeACKPacket( int seqnum, int acknum){ 
    struct pkt* packet = new pkt();
    packet->seqnum = seqnum;
    packet->acknum = acknum;
    memset(packet->payload,0,sizeof(packet->payload));
    packet->checksum = seqnum + acknum;
    return packet;
 }

void B_input(struct pkt packet){
  if (packet.checksum != checkSum(&packet)) {
    return;
  }
  if( packet.seqnum != b_nextseqnum){
      struct pkt *ack = makeACKPacket(packet.seqnum, 1);
      tolayer3(1, *ack);
      return;
  }else{
    tolayer5(1, packet.payload);
    struct pkt *ack = makeACKPacket(b_nextseqnum, 1);
    tolayer3(1, *ack);
    b_nextseqnum += 1;
  }
 }

void B_init(){
    b_nextseqnum = 0;
 }
