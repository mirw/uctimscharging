/*
  The UCT IMS Charging System
  Copyright (C) 2009 - University of Cape Town
  Vitalis Gavole Ozianyi <vitozy@crg.ee.uct.ac.za>
  Joyce Bertha Mwangama <joycebm@crg.ee.uct.ac.za>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef CHARGING_H
#define CHARGING_H

#define START 1
#define INTERIM 2
#define STOP 3
#define EVENT 4

#define YES  50
#define NO   51

#include "includes.h"
#include "ctf_rf/rf.h"
#include "ctf_ro/ro.h"
#include "iptv/uct_aiptv_server.h"
#include "iptv/includes.h"


typedef enum {
	SStart 		= 1,	
	Interim		= 2,	
	SStop 		= 3,	
	Event		= 4		
	
}message_state_t;

typedef enum{
	Running   = 100,
	Not	  = 101
}msg_timer_state;

typedef enum{
	Yes    =  50,
	No     =  51
}diameter_success;	



/** User clients representation */
typedef struct _a_client{
	struct _a_client *next;		/**< the next person in the client list */
	struct _a_client *prev;		/**< the previous person in the client list */
	void **ptr;						
	
	int credits;			/**< how much credit the person has - not used here */ 
	int onceoff_credit, persec_credit, perMbyte_credit;
	int used_crtdonce, used_crtdpsec, used_crtdpMb;
	int used_crtdonce_buf, used_crtdpsec_buf, used_crtdpMb_buf;
	int used_crtdonce_buf1, used_crtdpsec_buf1, used_crtdpMb_buf1;	
	int call_id;			/**< This is set to IMS call id je->cid */
	int call_did;			/* Dialog ID */
	char *subscriber;		
	int charge_type; 		/** online=1 offline=0 **/	
	//int expires;			
	int timeout;			/*** timeout ***/		
		
	diameter_success feedback;
	msg_timer_state clocking;
	message_state_t events;
}a_client;

/** Client list representation */
typedef struct {		
	gen_lock_t *lock;		/**< lock for list operations */				
	a_client *head,*tail;		/**< first, last clients in the list */ 	
		
}client_list;

typedef struct{
	gen_lock_t *lock;
}sip_lock;

void sip_timer(time_t now, void *ptr);
void msg_timer(time_t now, void *ptr);
int msg_timer_init(char *key_value_file);
int sip_timer_init();
inline int add_client(int timeout, int callID, char *subscriber, int charge_type, int did);
inline void del_client(int callID);
inline void free_client(a_client *this);
int get_charge_type_from_sdp(osip_message_t *request);
int charge_iptv(int charge_type, int charge_event, int cid, char *subscriber, int did);
int get_charge_type_from_a_client(int cal_ID);
int event_handler(int msg_type, int charging_type, int cid, char *subscriber, int crdtonce, int crdtpsec, int crdtpMb);




#endif

