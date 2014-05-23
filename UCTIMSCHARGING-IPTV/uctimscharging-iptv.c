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
   
  This file utilizes the C Diameter Peer from Fokus

*/
#include "main.h"
#include "charging.h"
#include "includes.h"

/** Configuration */

char *CDiameterPeer_config=0;
extern int debug;

char *keys_file = 0;



static void main_sig_handler(int signo)
{
	if ( getpid()==*dp_first_pid ) {
						
		switch (signo) {
			case 0: break; /* do nothing*/
			case SIGPIPE:
				LOG(L_CRIT, "WARNING: SIGPIPE received and ignored\n");
				break;
			case SIGINT:
			case SIGTERM:
				if (signo==SIGINT)
					DBG("SIGINT received, program terminates\n");
				else
					DBG("SIGTERM received, program terminates\n");
				diameter_peer_destroy();
				exit(0);
				break;
			case SIGHUP: /* ignoring it*/
				DBG("SIGHUP received, ignoring it\n");
				break;
			case SIGUSR1:
				LOG(memlog, "Memory status (shm):\n");
				shm_status();
				break;
			default:
				LOG(L_CRIT, "WARNING: unhandled signal %d\n", signo);
		}
	}
	return;
}

int main_set_signal_handlers()
{
	/** install signal handler */
	if (signal(SIGINT, main_sig_handler)==SIG_ERR) {
		LOG(L_ERR,"ERROR:main: cannot install SIGINT signal handler\n");
		goto error;
	}
	if (signal(SIGPIPE, main_sig_handler) == SIG_ERR ) {
		LOG(L_ERR,"ERROR:main: cannot install SIGPIPE signal handler\n");
		goto error;
	}
	if (signal(SIGUSR1, main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"ERROR:main: cannot install SIGUSR1 signal handler\n");
		goto error;
	}
	if (signal(SIGTERM , main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"ERROR:main: cannot install SIGTERM signal handler\n");
		goto error;
	}
	if (signal(SIGHUP , main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"ERROR:main: cannot install SIGHUP signal handler\n");
		goto error;
	}
	if (signal(SIGUSR2 , main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"ERROR:main: cannot install SIGUSR2 signal handler\n");
		goto error;
	}	
	
	return 1;
error:
	return 0;
}

/*
 * The main Function
 @Params
 * */

int main(int argc,char* argv[])
{	
	LOG(L_INFO,"Starting UCT IMS Charging System (CTF)\n");
 	LOG(L_INFO,"Developers: Vitalis Gavole Ozianyi and Joyce B. Mwangama (2009)\n");

	    
   debug = 3;
   CDiameterPeer_config = "uctimscharging.xml";
   keys_file = "key_value_file.xml"; 
    
    init_memory(0);
    main_set_signal_handlers();
   
   	 /** check if the configure xml file is valid*/
    if (!diameter_peer_init(CDiameterPeer_config)){
	LOG(L_CRIT,"CRITICAL:main () - Error on diameter_peer_init\n");
	return -1;
	} 

    if(!cb_add(process_incoming,0))
	{
	LOG(L_CRIT,"CRITICAL:main () - Error on cd_add\n");
	}
    
    	
	// The IPTV server is started in this function
	if(!msg_timer_init(keys_file)){
		LOG(L_CRIT,"ERROR: main() - Error starting IPTV, quiting...\n");
		goto quit;
	}
	
	if(!sip_timer_init()){
		LOG(L_CRIT,"ERROR: main() - Error starting SIP timer, quiting...\n");
		goto quit;
	}

	if (!diameter_peer_start(1)) 
    {
       	LOG(L_CRIT,"CRITICAL:main () - Error on diameter_peer_start\n");
		return -1;
	}
	
	LOG(L_INFO,"Charging system and IPTV Servers are ready to accept client requests...\n");
       
quit:
	diameter_peer_destroy();
		
	if (debug>1) destroy_memory(1);
	else destroy_memory(0);
		
	return 0;	
}



