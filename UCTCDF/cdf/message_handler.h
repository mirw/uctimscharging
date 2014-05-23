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

#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <time.h>
#include "rf_avp.h"
#include "../cdp/utils.h"
#include "../cdp/diameter.h"
#include "../cdp/diameter_api.h"
#include "../cdp/peer.h"
#include "../cdp/config.h"
#include "../includes.h"


typedef enum {
	SStart 		= 1,	
	Interim		= 2,	
	SStop 		= 3,	
	Event		= 4		
} message_state_t;

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
	
	int credits, credits_buf, used_credits, debits, debits_buf;	
	int onceoff_credit, persec_credit, perMbyte_credit;
	int used_crdtonce, used_crdtpsec, used_crdtpMb;
	int call_id;
	char *subscriber;		
	int charge_type; 		
	void **ptr;						
	int timeout;					
			
	//diameter_success feedback;
	msg_timer_state clocking;
	message_state_t events;
	
}a_client;

/** Client list representation */
typedef struct {		
	gen_lock_t *lock;		/**< lock for list operations */				
	a_client *head,*tail;		/**< first, last clients in the list */ 	
}client_list;


void msg_handler(char * subscriber, int msg_type, int *result, int cid, int *crdtonce, int *crdtpsec, int *crdtpMb, int used_crdtonce, int used_crdtpsec, int used_crdtpMb);
void msg_timer(time_t now, void *ptr);
int msg_timer_init();
inline int add_client(int timeout, int callID,  int charge_type, char *subscriber);
inline void del_client(int callID);
inline void free_client(a_client *this);

#endif
