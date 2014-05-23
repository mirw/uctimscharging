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

#include <stdlib.h>
#include <sys/wait.h> 
#include <signal.h>

#include "main.h"
#include "cdp/diameter_peer.h"
#include "server.h"
#include "cdp/worker.h"
#include "cdp/timer.h"
#include "cdp/globals.h"
#include "server.h"
#include "cdf/message_handler.h"
#include "includes.h"

/* Configuration */

char *CDiameterPeer_config=0;
extern int debug;

static void main_sig_handler(int signo)
{
	if ( getpid()==*dp_first_pid ) {
		/* I'am the main thread */
		switch (signo) {
			case 0: break; /* do nothing*/
			case SIGPIPE:
				LOG(L_INFO, "main_sig_handler: - WARNING: SIGPIPE received and ignored\n");
				break;
			case SIGINT:
			case SIGTERM:
				if (signo==SIGINT)
					DBG("main_sig_handler: - SIGINT received, program terminates\n");
				else
					DBG("main_sig_handler: - SIGTERM received, program terminates\n");
				diameter_peer_destroy();
				exit(0);
				break;
			case SIGHUP: /* ignoring it*/
				DBG("main_sig_handler: - SIGHUP received, ignoring it\n");
				break;
			case SIGUSR1:
				LOG(memlog, "main_sig_handler: - Memory status (shm):\n");
				shm_status();
				break;
			default:
				LOG(L_CRIT, "main_sig_handler: - WARNING: unhandled signal %d\n", signo);
		}
	}
	return;
}

int main_set_signal_handlers()
{
	/* install signal handler */
	if (signal(SIGINT, main_sig_handler)==SIG_ERR) {
		LOG(L_ERR,"main_set_signal_handlers: - ERROR:main: cannot install SIGINT signal handler\n");
		goto error;
	}
	
	if (signal(SIGPIPE, main_sig_handler) == SIG_ERR ) {
		LOG(L_ERR,"main_set_signal_handlers: - ERROR:main: cannot install SIGPIPE signal handler\n");
		goto error;
	}

	if (signal(SIGUSR1, main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"main_set_signal_handlers: - ERROR:main: cannot install SIGUSR1 signal handler\n");
		goto error;
	}

	if (signal(SIGTERM , main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"main_set_signal_handlers: - ERROR:main: cannot install SIGTERM signal handler\n");
		goto error;
	}

	if (signal(SIGHUP , main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"main_set_signal_handlers: - ERROR:main: cannot install SIGHUP signal handler\n");
		goto error;
	}

	if (signal(SIGUSR2 , main_sig_handler)  == SIG_ERR ) {
		LOG(L_ERR,"main_set_signal_handlers: - ERROR:main: cannot install SIGUSR2 signal handler\n");
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
	LOG(L_INFO,"Starting the UCT Offline Charging Function\n");
	LOG(L_INFO,"Developers: Vitalis Gavole Ozianyi and Joyce B. Mwangama (2009)\n");

	
	//if (argc<3){
	//	LOG(L_CRIT,"Debug level and/or Configuration file was not provided as parameter!\n"); 
	//	LOG(L_INFO,"Usage: main <debug-level> <cfg_filename> \n\n");
	//	return -1;
	//}
	
	//debug=atoi(argv[1]);
	debug = 3;
	//CDiameterPeer_config = argv[2];
	CDiameterPeer_config = "cdf.xml";
	
	init_memory(0);
				
	if(!main_set_signal_handlers())
	{
	LOG(L_ERR,"CDF: main() - cannot install signal handlers\n");
	return -1;
	}
	
	if (!diameter_peer_init(CDiameterPeer_config)){
		LOG(L_CRIT,"CRITICAL:Error on diameter_peer_init\n");
		return -1;
	}

	if(!cb_add(process_incoming,0)) return -1;

	if(!msg_timer_init()) 
	{
	LOG(L_ERR,"CDF: main() - Error on msg_timer_init\n");
	return -1;
	} 
	
	if (!diameter_peer_start(1)){ 
		LOG(L_CRIT,"CRITICAL:Error on diameter_peer_start\n");
		return -1;
	}
	
	diameter_peer_destroy();
	
	if (debug>1)
	destroy_memory(1);
	else
	destroy_memory(0);

	return 0;	
}



