#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>

#include <stdlib.h>
#include <time.h>


//#define START 1
//#define INTERIM 2
//#define STOP 3
//#define EVENT 4


#include "../cdp/peermanager.h"
#include "../cdp/timer.h"
#include "../cdp/utils.h"

#include "message_handler.h"

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
	//Always assign 1 credit for services that don't use it to prevent session shutdown by AS 	
	*result = 0;
	int onceoff_charge, persec_charge, perMbyte_charge, temp_credit;
	int time_cred_factor, vol_cred_factor;
	int dec_ratio;
	//int cred_dec_factor;
	//int tod_factor;	

	// Multipying factors for setting credit allocations 
	time_cred_factor = 60;	//Tries to authorize service for 60 seconds
	vol_cred_factor = 1;	//1 Mbyte
		
	// Credit decreament factor on insufficient credit 
	//cred_dec_factor = 2; 
	dec_ratio = 2; 	
	
	onceoff_charge = 1;	
	persec_charge = 1;	
	perMbyte_charge = 1; 
			
	// Time of day charging factor 
	//tod_factor = 1.2;	
	
	switch (msg_type)
	{
		case START:
			LOG(L_INFO,"START - CCR received for %s ID %d with values Once-off %d\tTime %d\tVol %d.\n",subscriber,cid,used_crdtonce,used_crdtpsec,used_crdtpMb);
			
			lock_get(list_of_clients->lock);
			
			if(!list_of_clients->head){
				LOG(L_DBG,"No clients Found on list.\n");
				LOG(L_DBG,"Adding new client with ID %d\n",cid);
				lock_release(list_of_clients->lock);
				if(!add_client(30,cid,1,subscriber)){
					LOG(L_ERR,"ERROR: msg_handler() - Failed to add client ID %d to list",cid); 
				}
			
			} else lock_release(list_of_clients->lock);
			
			lock_get(list_of_clients->lock);
			client = list_of_clients->head;
					
			while(client->call_id != cid && client->next !=0){
				//LOG(L_DBG,"Did We get here?\n");	
				client=client->next;
			}
			//LOG(L_DBG,"We definitely get here\n");	
			if(client->call_id != cid && client->next == 0){
				LOG(L_DBG,"Client does not exist.\n");
				LOG(L_DBG,"Adding new client with ID %d.\n",cid);
				lock_release(list_of_clients->lock);
				if(!add_client(30,cid,1,subscriber)){
					LOG(L_ERR,"ERROR: msg_handler() - Failed to add client ID %d to list",cid); 
				}
				lock_get(list_of_clients->lock);
				client = list_of_clients->head;

				while(client->call_id != cid && client->next !=0){
					LOG(L_DBG,"This client ID is %d\n",client->call_id); 
					client=client->next;
				}

			}

			if(client->call_id == cid){ 
				LOG(L_DBG,"Found %s with ID %d and credits %d.\n",client->subscriber,client->call_id,client->credits);	
				
				onceoff_charge = onceoff_charge;
				persec_charge = time_cred_factor * persec_charge; 
				perMbyte_charge = vol_cred_factor * perMbyte_charge; 
			
				if (client->credits > 0){
					while(client->credits - onceoff_charge - persec_charge - perMbyte_charge <= 0)
					{
						LOG(L_WARN,"START - Limited credit, proceeding with further checks. Credits %d\n",client->credits);
						if(persec_charge || perMbyte_charge > 0){				
							persec_charge = persec_charge / dec_ratio;
							perMbyte_charge = perMbyte_charge / dec_ratio;
						}
						else{
							LOG(L_WARN,"START - Insufficient credit for some services, proceeding with further checks. Credits %d\n",client->credits);
						}
					}

					temp_credit = client->credits - onceoff_charge;
					if (temp_credit > 0){
						client->credits = temp_credit;
						client->credits_buf = onceoff_charge;
						temp_credit = client->credits - persec_charge;
						if (temp_credit > 0){
							client->credits = temp_credit;
							client->credits_buf = client->credits_buf + persec_charge;
							temp_credit = client->credits - perMbyte_charge;
							if (temp_credit > 0){
								client->credits = temp_credit;
								client->credits_buf = client->credits_buf + perMbyte_charge;

								LOG(L_INFO,"START - Client requested service, recording of session will begin.\n");
						
								*crdtonce = onceoff_charge;
								*crdtpsec = persec_charge;
								*crdtpMb = perMbyte_charge;

								*result = 1;
								//client->events = SStart;
							}
							else{
								LOG(L_WARN,"START - Client has insufficient credit for Volume based charging. Credits %d\n",client->credits + onceoff_charge + persec_charge);
								// Return credits
								client->credits = client->credits + client->credits_buf;
								client->credits_buf = 0;
								// Set allocated credit to 0 		
								*crdtonce = 0;
								*crdtpsec = 0;
								*crdtpMb = 0;

								*result = 1;
								//client->events = SStop;
							}
						}
						else{
							LOG(L_WARN,"START - Client has insufficient credit for duration based charging. Credits %d\n",client->credits + onceoff_charge);
							client->credits = client->credits + client->credits_buf;
							client->credits_buf = 0;

							*crdtonce = 0;
							*crdtpsec = 0;
							*crdtpMb = 0;

							*result = 1;
							//client->events = SStop;
						}
					
					}
					else
					{
						LOG(L_WARN,"START - Client has insufficient credit for Any click. Credits %d .\n",client->credits);
						client->credits_buf = 0;			
						
						*crdtonce = 0;
						*crdtpsec = 0;
						*crdtpMb = 0;

						*result = 1;	
						//client->events = SStop;			
					}
				}
				else{
					LOG(L_WARN,"START - Client has insufficient credits for all services.\n");
					
					*crdtonce = 0;
					*crdtpsec = 0;
					*crdtpMb = 0;

					*result = 1;
					//client->events = SStop;
				}
			}  
			lock_release(list_of_clients->lock);  			
			
		break;
			
		case INTERIM:
			//Deduct the consumed credits and update balance
			LOG(L_INFO,"INTERIM - CCR received with values once-off %d\tTime %d\tVol %d.\n",used_crdtonce,used_crdtpsec,used_crdtpMb);
			
			lock_get(list_of_clients->lock);	
			client = list_of_clients->head;
			while(client->call_id != cid && client->next !=0){
				client=client->next;
			}
			
			if(client->call_id != cid && client->next ==0){
			LOG(L_ERR,"Client Not Found. Message received for unknown session\n");
			
			*crdtonce = 0;
			*crdtpsec = 0;
			*crdtpMb = 0;
			
			*result = 1;
			}
			
			if(client->call_id == cid){ 	
				client->used_crdtonce = used_crdtonce;
				client->used_crdtpsec = used_crdtpsec;
				client->used_crdtpMb = used_crdtpMb;
				client->credits_buf = client->credits_buf - client->used_crdtonce - client->used_crdtpsec - client->used_crdtpMb; //Deduct used credits from pre-allocated credits

				//Allocate new credits - we set them equal to used credits
				persec_charge = client->used_crdtpsec; 
				perMbyte_charge = client->used_crdtpMb;

				while(client->credits - persec_charge - perMbyte_charge <= 0){

					if(persec_charge || perMbyte_charge > 0){				
					persec_charge = persec_charge / dec_ratio;
					perMbyte_charge = perMbyte_charge / dec_ratio;
					}
					else{
					LOG(L_WARN,"INTERIM - Insufficient credit for some services, proceeding with further checks\n");
					}
				}

				temp_credit = client->credits - persec_charge;
				if (temp_credit > 0){
					client->credits = temp_credit;
					client->credits_buf = client->credits_buf + persec_charge;
					temp_credit = client->credits - perMbyte_charge;
						if (temp_credit > 0){
							client->credits = temp_credit;
							client->credits_buf = client->credits_buf + perMbyte_charge;

							LOG(L_INFO,"New credits assigned.\n");
						
							*crdtonce = 0;
							*crdtpsec = persec_charge;
							*crdtpMb = perMbyte_charge;
							
							*result = 1;
							//client->events = SStart;
						}
						else{
							LOG(L_WARN,"INTERIM - Client has insufficient credit for Volume based charging.\n");

							//client->credits = client->credits + perMbyte_charge;
							//client->credits_buf = 0;
							*crdtonce = 0;
							*crdtpsec = 0;
							*crdtpMb = 0;

							*result = 1;
							//client->events = SStart;
						}
					}
					else{
						LOG(L_WARN,"INTERIM - Client has insufficient credit for time based charging.\n");
						// Return credits
						//client->credits = client->credits + persec_charge;
						//client->credits_buf = 0;

						*crdtonce = 0;
						*crdtpsec = 0;
						*crdtpMb = 0;

						*result = 1;
						//client->events = SStart;
					}

			}
			lock_release(list_of_clients->lock);
		break;

		case STOP:
			//Deduct the consumed credits and update the balance
			LOG(L_INFO,"STOP - CCR received with values once-off %d\tTime %d\tVol %d.\n",used_crdtonce,used_crdtpsec,used_crdtpMb);

			lock_get(list_of_clients->lock);
			client = list_of_clients->head;
			while(client->call_id != cid && client->next !=0){
				client=client->next;
			}
			
			if(client->call_id == cid){ 	
				client->used_crdtonce = used_crdtonce;
				client->used_crdtpsec = used_crdtpsec;
				client->used_crdtpMb = used_crdtpMb;
				client->credits_buf = client->credits_buf - client->used_crdtonce - client->used_crdtpsec - client->used_crdtpMb; //Deduct used credits from pre-allocated credits
				client->credits = client->credits + client->credits_buf; //Restore unused credits

				*crdtonce = 0;
				*crdtpsec = 0;
				*crdtpMb = 0;

				*result = 1;

				//client->events = SStop;
				
			}

			LOG(L_INFO,"Session Closed, recording will end. Credits remaining %d \n",client->credits);
			lock_release(list_of_clients->lock);
			del_client(client->call_id);

		break;
		case EVENT:
			//not used
		break;
	}	

}


/** Adds a new client to list of clients being serviced
@Params
int timeout - 
int auto_drop -
int client_id - (int caller)
char *subscriber
a_client that - Struct a_client
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
	this->credits = 200;
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

/** deletes client from client list */
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

/** frees the client stuff*/
inline void free_client(a_client *this)
{
	if (this->ptr) shm_free(this->ptr);
	shm_free(this);
}


