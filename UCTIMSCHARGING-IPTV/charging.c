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

#include "charging.h"
#include "iptv/uct_aiptv_server.h"
#include "ctf_ro/ro.h"

//char *timer_states[]={"Running","Not"};

//char *timer_events[]={"Start","Interim","Stop","Event"};

client_list *list_of_clients=0;
a_client *client=0;	
sip_lock *a_sip_lock=0;

/*
 * Initializes the timer for Listening to SIP requests at the IPTV server
 * */
int sip_timer_init()
{
	a_sip_lock = shm_malloc(sizeof(sip_lock));
	if(!a_sip_lock){
		LOG_NO_MEM("shm",sizeof(sip_lock));
		return 0;
	}
	
	a_sip_lock->lock = lock_alloc();
	a_sip_lock->lock = lock_init(a_sip_lock->lock);

	if(!add_timer(1,0,&sip_timer,0))
	{
	LOG(L_ERR,"CTF - sip_timer_init() - Failed to adder timer");
	return 0;
	}
	return 1;
}

/*
 * Initializes the charging timer
 @Params
 char *key_value_file - the IPTV hash table configuration file
 * 
 * */
int msg_timer_init(char *key_value_file)
{
	LOG(L_INFO,"UCT IMS Charging System\n");
	LOG(L_INFO,"Developers: Vitalis Gavole Ozianyi and Joyce B. Mwangama (2009)\n");
	LOG(L_INFO,"INFO: msg_timer_init() - Initialize message timer\n");
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
		
	LOG(L_INFO,"INFO: msg_timer_init() - Calling the IPTV function\n\n");
	
	if(!iptv_start(key_value_file))
	{
	LOG(L_CRIT,"CRITICAL: msg_timer_init() - Error on iptv_start\n");
	return 0;
	}
	
return 1;
}

/* Function called by timer process every one second
 * It listens to incoming IPTV requests
 @Params
 time_t now - The current time reference in seconds
 void *ptr - Pointer to function to be called on return
*/

void sip_timer(time_t now, void *ptr)
{
	lock_get(a_sip_lock->lock);
	get_exosip_events();
	lock_release(a_sip_lock->lock);
}

/*  Function called by timer process every one second
It performs charging processes for every active session
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
	//For each client
	client = list_of_clients->head;
	while(client->next != 0){
		LOG(L_DBG,"Processing client %s ID %d\n",client->subscriber,client->call_id);
		switch (client->events){
			case SStart:
			client->timeout = client->timeout - 1;
			client->used_crtdpsec = client->used_crtdpsec + 1;
			client->used_crtdpMb = 0;
		
			if(client->charge_type == 1)
			client->onceoff_credit = client->onceoff_credit - 0;

			if(client->charge_type == 1)
			client->persec_credit = client->persec_credit - 1;

			if (client->persec_credit == 0){
				client->events = SStop;
			}

			if(client->charge_type == 1)
			client->perMbyte_credit = client->perMbyte_credit - 0;			

			if (client->perMbyte_credit == 0){
				client->events = SStop;
			}
			
			LOG(L_DBG,"DBG: msg_timer() - Timeout is: %d\n",client->timeout);
			if (client->timeout == 0){
				//client->used_crtdpsec_buf = client->used_crtdpsec - client->used_crtdpsec_buf;
				client->used_crtdpsec_buf = client->used_crtdpsec - client->used_crtdpsec_buf1;
				client->used_crtdpsec_buf1 = client->used_crtdpsec;
				client->events = Interim;
			}
							
			break;
		
			case Interim:
			client->timeout = 30;
			client->events = SStart;
			lock_release(list_of_clients->lock);
			event_handler(INTERIM,client->charge_type,client->call_id,client->subscriber,client->used_crtdonce,client->used_crtdpsec_buf,client->used_crtdpMb);
					
			break;
		
			case SStop:
			client->clocking = Not;	
			lock_release(list_of_clients->lock);
			client->used_crtdpsec_buf = client->used_crtdpsec - client->used_crtdpsec_buf1;
			event_handler(STOP,client->charge_type,client->call_id,client->subscriber,client->used_crtdonce,client->used_crtdpsec_buf,client->used_crtdpMb);	
			break;
		
			case Event:
						
			break;
		}
		client = client->next;
		
	}
	if(client->next == 0){
		LOG(L_DBG,"Processing last client %s ID %d\n",client->subscriber,client->call_id);
		switch (client->events){
			case SStart:
			client->timeout = client->timeout - 1;
			client->used_crtdpsec = client->used_crtdpsec + 1;
			client->used_crtdpMb = 0;
		
			if(client->charge_type == 1)
			client->onceoff_credit = client->onceoff_credit - 0;

			if(client->charge_type == 1)
			client->persec_credit = client->persec_credit - 1;

			if (client->persec_credit == 0){
				client->used_crtdpsec_buf = client->used_crtdpsec - client->used_crtdpsec_buf;
				client->events = SStop;
			}
			
			if(client->charge_type == 1)
			client->perMbyte_credit = client->perMbyte_credit - 0;			

			if (client->perMbyte_credit == 0){
				client->events = SStop;
			}
			
			LOG(L_DBG,"DBG: msg_timer() - Timeout is: %d\n",client->timeout);
			if (client->timeout == 0){
				client->used_crtdpsec_buf = client->used_crtdpsec - client->used_crtdpsec_buf1;
				client->used_crtdpsec_buf1 = client->used_crtdpsec;
				client->events = Interim;
			}					
			break;

			case Interim:
			client->timeout = 30;
			client->events = SStart;
			lock_release(list_of_clients->lock);
			event_handler(INTERIM,client->charge_type,client->call_id,client->subscriber,client->used_crtdonce,client->used_crtdpsec_buf,client->used_crtdpMb);
					
			break;
		
			case SStop:
			client->clocking = Not;	
			client->used_crtdpsec_buf = client->used_crtdpsec - client->used_crtdpsec_buf1;
			lock_release(list_of_clients->lock);
			event_handler(STOP,client->charge_type,client->call_id,client->subscriber,client->used_crtdonce,client->used_crtdpsec_buf,client->used_crtdpMb);
			break;
		
			case Event:
		
			break;
		}
	}
	if(client->events != Interim || client->events != SStop)				
	lock_release(list_of_clients->lock);
	}
}


/*
This function is called by the IPTV server to trigger the charging process 
@Params
int charge_type - charging type 1=online and 0=offline
int charge_event - the charging event to be perfomed, e.g., START or STOP charging
int cid - a locally significant charging ID (je->cid
char *subscriber - user name, e.g., Alice
int did - the SIP dialog ID
*/
int charge_iptv(int charge_type, int charge_event, int cid, char *subscriber, int did)
{
	LOG(L_INFO,"charge_iptv - Calling the charging function.\n");
	
	int ct = charge_type; 
   	if(charge_event == START)
	{
     	if(!add_client(30,cid,subscriber,ct,did))
		{
			LOG(L_ERR,"ERROR: charge_iptv - START - Failed to add client ID %d to list",cid); 
		}

        if (!event_handler(START,ct,cid,subscriber,0,0,0))
		{
			LOG(L_ERR,"ERROR: charge_iptv - START - Event handler failed\n"); 
			return 0;  
		}
		
		lock_get(list_of_clients->lock);
		if(!list_of_clients->head){
			LOG(L_DBG,"charge_iptv START - No clients Found on list.\n");
			lock_release(list_of_clients->lock);
			return 0;
		}			
		client = list_of_clients->head;
		while(client->call_id != cid && client->next !=0){
		client=client->next;
		}

		if(client->call_id != cid && client->next == 0){
			LOG(L_ERR,"charge_iptv START - Client not found.\n");
			lock_release(list_of_clients->lock);
			return 0;
		}
		LOG(L_INFO,"charge_iptv START - Client %s ID %d\n",client->subscriber,client->call_id);
					 
       	client->feedback = NO;
        client->clocking = Running;
        client->events = SStart;
		lock_release(list_of_clients->lock);
		return 1;
			
	}
	else
	if(charge_event == STOP)
	{
		lock_get(list_of_clients->lock);
		if(!list_of_clients->head){
			LOG(L_DBG,"charge_iptv STOP - No clients Found on list.\n");
			lock_release(list_of_clients->lock);
			return 0;
		}			
		client = list_of_clients->head;
		while(client->call_id != cid && client->next !=0){
		client=client->next;
		}

		if(client->call_id != cid && client->next == 0){
			LOG(L_ERR,"charge_iptv STOP - Client not found.\n");
			lock_release(list_of_clients->lock);
			return 0;
		}
		LOG(L_INFO,"charge_iptv STOP - Found client %s call ID %d\n",client->subscriber,client->call_id);
        		
		client->used_crtdpsec_buf = client->used_crtdpsec - client->used_crtdpsec_buf;
		client->events = SStop;
		lock_release(list_of_clients->lock);						
		return 1;
	}
return 0;
}

/* Prepares and sends request to the appropriate charging function.
@Params
int msg_type - charging event type, e.g. START, INTERIM and STOP
int charging_type - online=1, offline=0
int cid - call ID, retrieved from je->cid
char *subscriber - subscriber name, e.g., Alice
int crdtonce - onceoff credit value
int crdtpsec - duration based credit value
int crdtpMb - volume based credit value
*/

int event_handler(int msg_type, int charging_type, int cid, char *subscriber, int crdtonce, int crdtpsec, int crdtpMb)
{
AAAMessage *acr; //Offline charging
AAAMessage *ccr; //Online charging

    int sesid = cid;
    str user_subscriber; user_subscriber.s = subscriber; user_subscriber.len = strlen(user_subscriber.s);  
    //sprintf(buf,"%d",cid);
    //str sesid; sesid.s = cid;sesid.len = strlen(sesid.s); 
    str ohost; ohost.s = "iptv.open-ims.test"; ohost.len = strlen(ohost.s);
    str orealm; orealm.s = "open-ims.test"; orealm.len = strlen(orealm.s);
    str drealm; drealm.s = "open-ims.test"; drealm.len = strlen(drealm.s); 
    str dhost; dhost.s = "cdf.open-ims.test"; dhost.len = strlen(dhost.s); 
    str dhost1; dhost1.s = "ocf.open-ims.test"; dhost1.len = strlen(dhost1.s);
    str acctype; acctype.s ="START"; acctype.len = strlen(acctype.s); 
    str accnumber; accnumber.s = "10"; accnumber.len = strlen(accnumber.s);
    str auth_app_id; auth_app_id.s = "100"; auth_app_id.len = strlen(auth_app_id.s);
    str service_context_id; service_context_id.s = "100"; service_context_id.len = strlen(service_context_id.s);
    str cc_type; cc_type.s ="START"; cc_type.len = strlen(cc_type.s);
    str cc_number; cc_number.s = "10"; cc_number.len = strlen(cc_number.s);
    
    switch (msg_type)
	{
		case START:
												
			if (charging_type == 0)
			{
				LOG(L_INFO,"INFO: event_handler - Invoking offline charging\n");
				
				lock_get(list_of_clients->lock);
				if(!list_of_clients->head){
					LOG(L_DBG,"event_handler START - No clients Found on list.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}			
				client = list_of_clients->head;
				while(client->call_id != cid && client->next !=0){
				client=client->next;
				}

				if(client->call_id != cid && client->next == 0){
					LOG(L_ERR,"event_handler START - Client not found.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}
				LOG(L_DBG,"event_handler START - Found client %s call ID %d\n",client->subscriber,client->call_id);
				lock_release(list_of_clients->lock);  
		
				LOG(L_INFO,"event_handler START - ACR (INITIAL_REQUEST).\n");
				acr = Rf_ACR(sesid,user_subscriber,ohost,orealm,drealm,START,accnumber,dhost,0,0,0);
				if(acr==0) return 0;
				
						
			}
			else if (charging_type == 1)
			{
				LOG(L_INFO,"INFO: event_handler - START - Invoking online charging\n");
					
					lock_get(list_of_clients->lock);
					if(!list_of_clients->head){
						LOG(L_DBG,"event_handler START - No clients Found on list.\n");
						lock_release(list_of_clients->lock);
						return 0;
					}			
					client = list_of_clients->head;
					while(client->call_id != cid && client->next !=0){
						client=client->next;
					}

					if(client->call_id != cid && client->next == 0){
						LOG(L_ERR,"event_handler START - Client not found.\n");
						lock_release(list_of_clients->lock);
						return 0;
					}
					LOG(L_DBG,"event_handler START - Found client %s call ID %d\n",client->subscriber,client->call_id);
				
					LOG(L_DBG,"event_handler START - Found %s callID %d and credits %d.\n",client->subscriber,client->call_id,client->onceoff_credit + client->persec_credit + client->perMbyte_credit);			
					lock_release(list_of_clients->lock);
					LOG(L_INFO,"event_handler START - CCR (INITIAL_REQUEST).\n");
					ccr = Ro_CCR(sesid,user_subscriber,ohost,orealm,auth_app_id,service_context_id,START,cc_number,dhost1,0,0,0);
					if(ccr==0) return 0;
					
			}
			
		break;
			
		case INTERIM: 
			if(charging_type == 0)
			{
				
				lock_get(list_of_clients->lock);
				if(!list_of_clients->head){
					LOG(L_DBG,"event_handler INTERIM - No clients Found on list.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}			
				client = list_of_clients->head;
				while(client->call_id != cid && client->next !=0){
					client=client->next;
				}

				if(client->call_id != cid && client->next == 0){
					LOG(L_ERR,"event_handler INTERIM - Client not found.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}
				LOG(L_DBG,"event_handler INTERIM - Found client %s call ID %d\n",client->subscriber,client->call_id);
				LOG(L_DBG,"event_handler INTERIM - Found %s callID %d and credits %d.\n",client->subscriber,client->call_id,client->onceoff_credit + client->persec_credit + client->perMbyte_credit);			
				lock_release(list_of_clients->lock); 
				LOG(L_INFO,"event_handler INTERIM - Sending ACR(UPDATE_REQUEST).\n");
				acr = Rf_ACR(sesid,user_subscriber,ohost,orealm,drealm,INTERIM,accnumber,dhost,crdtonce,crdtpsec,crdtpMb);
				if(acr==0) return 0;
					
			}
			else
			if(charging_type == 1)
			{
				
				lock_get(list_of_clients->lock);
				if(!list_of_clients->head){
					LOG(L_DBG,"event_handler INTERIM - No clients Found on list.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}			
				client = list_of_clients->head;
				while(client->call_id != cid && client->next !=0){
					client=client->next;
				}
				if(client->call_id != cid && client->next == 0){
					LOG(L_ERR,"event_handler INTERIM - Client not found.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}
				LOG(L_DBG,"event_handler INTERIM - Found client %s call ID %d\n",client->subscriber,client->call_id);
				LOG(L_DBG,"event_handler INTERIM - Found %s callID %d and credits %d.\n",client->subscriber,client->call_id,client->onceoff_credit + client->persec_credit + client->perMbyte_credit);			
				lock_release(list_of_clients->lock);
				LOG(L_INFO,"event_handler INTERIM - Sending CCR(UPDATE_REQUEST).\n");
				ccr = Ro_CCR(sesid,user_subscriber,ohost,orealm,auth_app_id,service_context_id,INTERIM,cc_number,dhost1,crdtonce,crdtpsec,crdtpMb);
				if(ccr==0) return 0;
			
			}
		break;
		
		case STOP:
					
			if(charging_type == 0)
			{
			 	lock_get(list_of_clients->lock);
				if(!list_of_clients->head){
					LOG(L_DBG,"event_handler STOP - No clients Found on list.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}			
				client = list_of_clients->head;
				while(client->call_id != cid && client->next !=0){
					client=client->next;
				}
				if(client->call_id != cid && client->next == 0){
					LOG(L_ERR,"event_handler STOP - Client not found.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}
				LOG(L_DBG,"event_handler STOP - Found client %s call ID %d\n",client->subscriber,client->call_id);
			 	LOG(L_DBG,"event_handler STOP - Found %s callID %d and credits %d.\n",client->subscriber,client->call_id,client->onceoff_credit + client->persec_credit + client->perMbyte_credit);
				lock_release(list_of_clients->lock);			
				LOG(L_INFO,"event_handler STOP - Sending ACR(TERMINATE_REQUEST).\n");
				acr = Rf_ACR(sesid,user_subscriber,ohost,orealm,drealm,STOP,accnumber,dhost,crdtonce,crdtpsec,crdtpMb);
				if(acr==0) return 0;
				del_client(cid);
			}	
			else
			if(charging_type == 1)
			{
				lock_get(list_of_clients->lock);
				if(!list_of_clients->head){
					LOG(L_DBG,"event_handler STOP - No clients Found on list.\n");
					lock_release(list_of_clients->lock);	
					return 0;
				}			
				client = list_of_clients->head;
				while(client->call_id != cid && client->next !=0){
					client=client->next;
				}

				if(client->call_id != cid && client->next == 0){
					LOG(L_ERR,"event_handler STOP - Client not found.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}
				LOG(L_DBG,"event_handler STOP - Found client %s call ID %d\n",client->subscriber,client->call_id);
				LOG(L_DBG,"event_handler STOP - Found %s callID %d and credits %d.\n",client->subscriber,client->call_id,client->onceoff_credit + client->persec_credit + client->perMbyte_credit);			
				lock_release(list_of_clients->lock);
				LOG(L_INFO,"event_handler STOP - Sending CCR(TERMINATE_REQUEST).\n");
				ccr = Ro_CCR(sesid,user_subscriber,ohost,orealm,auth_app_id,service_context_id,STOP,cc_number,dhost1,crdtonce,crdtpsec,crdtpMb);
				if(ccr==0) return 0;

				call_terminate(client->call_id,client->call_did);
				del_client(cid);
			}
			
		break;
		
		case EVENT:
				//Not yet used
				lock_get(list_of_clients->lock);
				if(!list_of_clients->head){
					LOG(L_DBG,"event_handler EVENT - No clients Found on list.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}			
				client = list_of_clients->head;
				while(client->call_id != cid && client->next !=0){
					client=client->next;
				}

				if(client->call_id != cid && client->next == 0){
					LOG(L_ERR,"event_handler EVENT - Client not found.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}

				LOG(L_DBG,"event_handler EVENT - Found client %s call ID %d\n",client->subscriber,client->call_id);
				LOG(L_DBG,"event_handler EVENT - Found %s callID %d and credits %d.\n",client->subscriber,client->call_id,client->onceoff_credit + client->persec_credit + client->perMbyte_credit);		
				client->events = Event;
				lock_release(list_of_clients->lock);
						
		break;
		case YES:
		
				lock_get(list_of_clients->lock);
				if(!list_of_clients->head){
					LOG(L_DBG,"event_handler YES - No clients Found on list.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}			
				client = list_of_clients->head;
				while(client->call_id != cid && client->next !=0){
					client=client->next;
				}

				if(client->call_id != cid && client->next == 0){
					LOG(L_ERR,"event_handler YES - Client not found.\n");
					lock_release(list_of_clients->lock);
					return 0;
				}
				client->subscriber = subscriber;//Temporary hack
				LOG(L_DBG,"event_handler YES - Found client %s call ID %d\n",client->subscriber,client->call_id);
				
				LOG(L_DBG,"event_handler YES - Found %s callID %d and credits %d.\n",client->subscriber,client->call_id,client->onceoff_credit + client->persec_credit + client->perMbyte_credit);			
				LOG(L_DBG,"event_handler YES -  Credits received for %s client ID: %d\tonce: %d\tTime: %d\tVol: %d\n",client->subscriber,client->call_id,crdtonce,crdtpsec,crdtpMb);

				if(client->charge_type == 1){			
					client->onceoff_credit = client->onceoff_credit + crdtonce;
					client->persec_credit = client->persec_credit + crdtpsec;
					client->perMbyte_credit = client->perMbyte_credit + crdtpMb;
					client->used_crtdonce = client->onceoff_credit;
					client->onceoff_credit = 0;
				}else{
					client->onceoff_credit = crdtonce;
					client->persec_credit = crdtpsec;
					client->perMbyte_credit = crdtpMb;
				}

				client->feedback = Yes;
				lock_release(list_of_clients->lock);
							
		break;
		
		case NO:
		//Not used

		break;
	}
	return 1;
}

/** Looks for client and adds a new client to list of clients if it doesnt exist
@Params
int cid - caller ID
char *subscriber
Returns - 0 on failure and 1 on success
*/
/*a_client *seek_client(int cid, char *subscriber, int charge_type)
{
	a_client *this;
	lock_get(list_of_clients->lock);
//Find client by callID
	if(!list_of_clients->head){
		LOG(L_INFO,"No clients Found on list.\n");
		//LOG(L_INFO,"Adding new client with ID %d\n",cid);

		return 0;			
	}
			
	this = list_of_clients->head;
				
	while(this->call_id != cid && this->next !=0){
		this=this->next;
	}

	//if the client does not exist
	if(this->call_id != cid && this->next == 0){
		LOG(L_ERR,"seek_client: Client not found.\n");
		return 0;
	}
	//client->subscriber = subscriber;//Temporary hack
	LOG(L_INFO,"Found client %s call ID %d\n",client->subscriber,client->call_id);
	//return 1;
	lock_release(list_of_clients->lock);
	return this;	
}
*/

/** Adds a new client to list of clients being serviced
@Params
int timeout - Interim period
int callID - je->cid used to identify the subscriber being processed
char *subscriber - subscriber name, e.g., Alice
int cherge_type - online or offline charging (1 or 0)
int did - SIP dialog ID
Returns - 0 on failure and 1 on success
*/
inline int add_client(int timeout, int callID, char *subscriber, int charge_type, int did)
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
	
	this->next = 0;
	this->call_id = callID;
	this->call_did = did;
	this->credits = 0;
	this->timeout = timeout;
	this->clocking = Not;
	this->subscriber = subscriber;
	this->charge_type = charge_type;
	this->onceoff_credit = 0;
	this->persec_credit = 0;
	this->perMbyte_credit = 0;
	this->used_crtdonce = 0;
	this->used_crtdpsec = 0;
	this->used_crtdpMb = 0;
	this->used_crtdonce_buf = 0;
	this->used_crtdpsec_buf = 0;
	this->used_crtdpMb_buf = 0;
	this->used_crtdonce_buf1 = 0;
	this->used_crtdpsec_buf1 = 0;
	this->used_crtdpMb_buf1 = 0;
		
	lock_get(list_of_clients->lock);
	this->prev = list_of_clients->tail;

	if(list_of_clients->tail){
		LOG(L_DBG,"add_client - Clients exist at tail\n");
		list_of_clients->tail->next =this;
	}
	list_of_clients->tail = this;
	
	if (!list_of_clients->head) {
		LOG(L_DBG,"add_client - No client at head means no clients in the list\n");
		list_of_clients->head = this;
	}
	lock_release(list_of_clients->lock);
	LOG(L_DBG,"add_client - We added client Name %s with callID %d\n",this->subscriber,this->call_id);
	
	return 1;
}

/** deletes client from client list 
 @Params
 int callID - client->callID (je->cid) used to identify the user being processed
 * */
inline void del_client(int callID)
{
	a_client *this;
	lock_get(list_of_clients->lock);
	this = list_of_clients->head;
	LOG(L_DBG,"This client's callID is %d\n",this->call_id);
	while(this->call_id != callID && this->next !=0)
	{
		this = this->next; //goto next client in list
		LOG(L_DBG,"This client's callID is %d\n",this->call_id);
	}

	if (this->call_id == callID){
		LOG(L_DBG,"Deleting client callID %d\n",this->call_id);
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

/** frees the memory used by a client
 @Params
 a_client this - the client to be freed
 * */
inline void free_client(a_client *this)
{
	if (this->ptr) shm_free(this->ptr);
	shm_free(this);
}

/*
* Get charging type (online or offline) from SDP body of message
* Only useful for IMS clients that allow users to define preferred charging options
@Params 
osip_message_t *request - SIP request message
**/

int get_charge_type_from_sdp(osip_message_t *request)
{
	sdp_message_t *remote_sdp;
  	sdp_media_t *remote_med;
  	int pos;
  	int ch_type;

	sdp_message_init(&remote_sdp);
	remote_sdp = eXosip_get_sdp_info (request);
	 	
	pos = 0;

  	while (!osip_list_eol (&remote_sdp->m_medias, pos))
  		{	
    		char payloads[128];
    		int pos3 = 0;
      			
    		memset (payloads, '\0', sizeof (payloads));
    		remote_med = (sdp_media_t *) osip_list_get (&remote_sdp->m_medias, pos);
      			
			pos3 = 0;
			sdp_attribute_t *a_attrib;
			
			while (!osip_list_eol (&remote_med->a_attributes, pos3))
				{
					a_attrib = osip_list_get (&remote_med->a_attributes, pos3);
					if (strstr(a_attrib->a_att_field, "ct"))
						{
							if (strstr(a_attrib->a_att_value,"1"))
								{
									ch_type=1;
									LOG(L_INFO,"Online charging requested\n");
								}
								
								if (strstr(a_attrib->a_att_value,"0"))
								{
									ch_type=2;
									LOG(L_INFO,"Offline charging requested\n");
								}
						}
					else
					ch_type = 1; /*  This is where the default charging type is defined */									
					pos3++;						
				}
      			
      		pos++;
		}
	return ch_type;	
}


/*
* Gets charging type from client container
@Params
int call_ID - client->call_ID (je->cid) ID used to identify the client
**/

int get_charge_type_from_a_client(int call_ID)
{
	int sch_type;
		
	lock_get(list_of_clients->lock);
	if(!list_of_clients->head){
		LOG(L_DBG,"get_charge_type_from_a_client - No clients Found on list.\n");
		lock_release(list_of_clients->lock);
		return 0;
	}			
	client = list_of_clients->head;
	while(client->call_id != call_ID && client->next !=0){
		client=client->next;
	}
	//if the client does not exist
	if(client->call_id != call_ID && client->next == 0){
		LOG(L_ERR,"get_charge_type_from_a_client - Client not found.\n");
		lock_release(list_of_clients->lock);
		return 0;
	}
	sch_type = client->charge_type;
	lock_release(list_of_clients->lock);			
	if(sch_type == 0) sch_type=2;
	return sch_type;
}
