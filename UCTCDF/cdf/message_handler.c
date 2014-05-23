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

#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>

#include <stdlib.h>
#include <time.h>

#define START 1
#define INTERIM 2
#define STOP 3
#define EVENT 4

#include "message_handler.h"
#include "../cdp/peermanager.h"
#include "../cdp/timer.h"
#include "../cdp/utils.h"

//char *timer_states[]={"Running","Not"};

//char *timer_events[]={"Start","Interim","Stop","Event"};

//msg_timer_list *msg_time=0;

client_list *list_of_clients=0;	

a_client *client=0;	

/****************
msg_timer_init()
This function initializes the list of clients

Return - 1 on success 0 on error
****************/


int msg_timer_init()
{
	LOG(L_INFO,"INFO: msg_timer_init() - Initializing the client list.\n");

	list_of_clients = shm_malloc(sizeof(client_list));
	if (!list_of_clients){
		LOG_NO_MEM("shm",sizeof(client_list));
		return 0;
	}
	
	list_of_clients->head = 0;
	list_of_clients->tail = 0;
	
	list_of_clients->lock = lock_alloc();
	list_of_clients->lock = lock_init(list_of_clients->lock);

	if(!add_timer(1,0,&msg_timer,0))
	{
	LOG(L_ERR,"CTF - msg_timer_init() - Failed to adder timer"); 
	return 0;
	}
		
	return 1;

}

/*  Function called by timer process every one second
It performs charging processes for every active user
* It basically checks if the Interim period has been exceeded
* Further functionality may be incorporated
@Params
time_t now - The current time reference in seconds
void *ptr - Pointer to function to be called on return
*/

void msg_timer(time_t now, void *ptr)
{		
	lock_get(list_of_clients->lock);
	
	if(!list_of_clients->head){
		LOG(L_DBG,"No clients, nothing to do\n");
		lock_release(list_of_clients->lock);
	}
	else{
	client = list_of_clients->head;
	while(client->next != 0){
		LOG(L_DBG,"This is not the last client - Processing client ID %d\n",client->call_id);
		switch (client->events){
			case SStart:
			client->timeout = client->timeout - 1;
			LOG(L_DBG,"DBG: msg_timer() - Timeout is: %d\n",client->timeout);
			if (client->timeout == 0){
				LOG(L_WARN,"Timeout reached without INTERIM Reports\n");
			}
							
			break;
		
			case Interim:
			client->timeout = 30;
			client->events = SStart;
					
			break;
		
			case SStop:
			break;
		
			case Event:
		
			break;
		
		}
	client = client->next;
	}
	if(client->next == 0){
		LOG(L_DBG,"This is the last client - Processing client ID %d\n",list_of_clients->head->call_id);
		switch (client->events){
			case SStart:
			client->timeout = client->timeout - 1;
			
			LOG(L_DBG,"DBG: msg_timer() - Timeout is: %d\n",client->timeout);
			if (client->timeout == 0){
				LOG(L_WARN,"Timeout reached without INTERIM Reports\n");
			}
							
			break;
		
			case Interim:
			client->timeout = 30;
			client->events = SStart;
					
			break;
		
			case SStop:
			break;
		
			case Event:
		
			break;
		
		}
	}
	
	lock_release(list_of_clients->lock);	

	}	
}

/*  Function that processes the Diameter accounting requests
@Params
* subscriber - user name, e.g. Alice
* msg_type - the charging event, e.g., START, INTERIM, STOP
* result - a conditional varriable
* cid - The charging ID
* crdtonce - Credits allocated for once-off charge events
* crdtpsec - Credits allocated for duration based charging
* crdtpMb - Credits allocated for volume based charging
* used_crdtonce - Credits consumed by once-off charging events
* used_crdtpsec - Credits consumed by duration based charging
* used_crdtpMb -  Credits consumed by volume based charging

*/
void msg_handler(char * subscriber, int msg_type, int *result, int cid, int *crdtonce, int *crdtpsec, int *crdtpMb, int used_crdtonce, int used_crdtpsec, int used_crdtpMb)
{
	*result = 0;
		
	switch (msg_type)
	{
		case START:
			LOG(L_INFO,"START - ACR received for %s with values Once-off %d\tTime %d\tVol %d.\n",subscriber,used_crdtonce,used_crdtpsec,used_crdtpMb);

			if(!list_of_clients->head){
				LOG(L_DBG,"No clients Found on list.\n");
				LOG(L_DBG,"Adding new client with ID %d\n",cid);
			
				if(!add_client(30,cid,1,subscriber)){
					LOG(L_ERR,"ERROR: msg_handler() - Failed to add client ID %d to list",cid); 
				}
			
			}
			
			client = list_of_clients->head;
						
			while(client->call_id != cid || client->next !=0){
				client=client->next;
			}

			if(client->call_id != cid && client->next == 0){
				LOG(L_DBG,"Client does not exist.\n");
				LOG(L_DBG,"Adding new client with ID %d.\n",cid);
			
				if(!add_client(30,cid,1,subscriber)){
					LOG(L_ERR,"ERROR: msg_handler() - Failed to add client ID %d to list",cid); 
				}

				client = list_of_clients->head;

				while(client->call_id != cid || client->next !=0){
					LOG(L_DBG,"This client ID is %d\n",client->call_id); 
					client=client->next;
				}

			}
			if(client->call_id == cid){ 
				LOG(L_DBG,"Found client with ID %d and credits %d.\n",client->call_id,client->credits);
				if (client->credits > 0){
					LOG(L_INFO,"START - Service usage for %s has been authorized.\n",client->subscriber);

					*crdtonce = 1; //values set to 1 for CTF operation
					*crdtpsec = 1;
					*crdtpMb = 1;

					*result = 1;
					client->events = SStart;
				}
				else{
					LOG(L_WARN,"START - %s has not been authorized to use the requested service\n",client->subscriber);
					// Set allocated credit to 0 		
					*crdtonce = 0;
					*crdtpsec = 0;
					*crdtpMb = 0;

					*result = 1;
					client->events = SStop;
				}
			}			
		break;
			
		case INTERIM:
			LOG(L_INFO,"INTERIM - ACR received with values once-off %d\tTime %d\tVol %d.\n",used_crdtonce,used_crdtpsec,used_crdtpMb);

			client = list_of_clients->head;
			while(client->call_id != cid || client->next !=0){
				client=client->next;
			}
			
			if(client->call_id == cid){ 	
				client->used_crdtonce = used_crdtonce;
				client->used_crdtpsec = used_crdtpsec;
				client->used_crdtpMb = used_crdtpMb;
				client->debits_buf = client->debits_buf + client->used_crdtonce + client->used_crdtpsec + client->used_crdtpMb; //Add used credits to debit account

				if (client->credits > 0){
					LOG(L_INFO,"INTERIM - Service usage for %s has been authorized.\n",client->subscriber);
					// Set allocated credit to AVP 
					*crdtonce = 1; //values set to 1 for CTF operation
					*crdtpsec = 1;
					*crdtpMb = 1;

					*result = 1;
					client->events = INTERIM;
				}
				else{
					LOG(L_WARN,"INTERIM - %s has not been authorized to use the requested service\n",client->subscriber);

					*crdtonce = 0;
					*crdtpsec = 0;
					*crdtpMb = 0;

					*result = 1;
					client->events = SStop;
				}
			}			


		break;

		case STOP:
			LOG(L_INFO,"STOP - ACR received with values once-off %d\tTime %d\tVol %d.\n",used_crdtonce,used_crdtpsec,used_crdtpMb);

			client = list_of_clients->head;
			while(client->call_id != cid || client->next !=0){
				client=client->next;
			}
			
			if(client->call_id == cid){ 	
				client->used_crdtonce = used_crdtonce;
				client->used_crdtpsec = used_crdtpsec;
				client->used_crdtpMb = used_crdtpMb;
				
				client->debits_buf = client->debits_buf + client->used_crdtonce + client->used_crdtpsec + client->used_crdtpMb; //Add used credits to debit account
				client->debits = client->debits + client->debits_buf; //Sum used credits
				
				// Set allocated credit to 0			
				*crdtonce = 0;
				*crdtpsec = 0;
				*crdtpMb = 0;

				*result = 1;
				client->events = SStop;
				
			}

			LOG(L_INFO,"Session Closed. Credits consumed by: %s are: %d \n",client->subscriber,client->credits);
			//Delete client
			del_client(client->call_id);			
		break;
	
		case EVENT:
			//not used
			LOG(L_INFO,"EVENT - ACR received.\n");	

			*crdtpsec = 0;
			*crdtpMb = 0;
			client->events = Event;
		
			*result = 1;
		
		break;
	}
}


/* Adds a new client to list of clients being serviced
@Params
timeout - INTERIM report timeout
callID - charging ID
charge_type - offline or online charging
subscriber - user name, e.g. Alice
Returns - 0 on failure and 1 on success
*/
inline int add_client(int timeout, int callID, int charge_type, char *subscriber)
{
	a_client *this;
	this = shm_malloc(sizeof(a_client));
	if (!this) {
		LOG_NO_MEM("shm",sizeof(a_client));
		return 0;
	}

	this->ptr = shm_malloc(sizeof(void*));
	if (!this->ptr) {
		LOG_NO_MEM("shm",sizeof(void*));
		shm_free(this);
		return 0;
	}
	
	this->subscriber = subscriber;
	this->next = 0;
	this->call_id = callID;
	this->credits = 3;
	this->timeout = 30;
	this->next = 0;
	this->charge_type = charge_type;
		
	lock_get(list_of_clients->lock);
	this->prev = list_of_clients->tail;

	if(list_of_clients->tail){
		LOG(L_DBG,"Clients exist at tail\n");
		list_of_clients->tail->next =this;
	}
	list_of_clients->tail = this;
	
	if (!list_of_clients->head) {
		LOG(L_DBG,"No client at head means no clients in the list\n");
		list_of_clients->head = this;
	}
		
	lock_release(list_of_clients->lock);
	LOG(L_DBG,"We added client ID %s with callID %d\n",this->subscriber,this->call_id);
	
	return 1;
}

/* deletes client from client list 
 @Params
 callID - charging ID
 * */
inline void del_client(int callID)
{
	a_client *this;
	lock_get(list_of_clients->lock);
	this = list_of_clients->head;
	LOG(L_DBG,"This client's callID is %d\n",this->call_id);
	while(this->call_id != callID && this->next !=0)
	{
		this = this->next; 
		LOG(L_DBG,"This client's callID is %d\n",this->call_id);
	}

	if (this->call_id == callID){
		if(list_of_clients->head->call_id != callID)
		this->prev->next = this->next;
		else list_of_clients->head = this->next;
		if (this->next != 0)
		this->next->prev = this->prev;
		else list_of_clients->tail = this->prev;
		free_client(this);
	}
	lock_release(list_of_clients->lock);
}

/*Frees memory used by client 
 @Params
 this - client 
 * */
inline void free_client(a_client *this)
{
	if (this->ptr) shm_free(this->ptr);
	shm_free(this);
}




