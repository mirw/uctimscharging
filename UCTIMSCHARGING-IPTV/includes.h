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

#ifndef INCLUDES_H
#define INCLUDES_H

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <regex.h>

#include <eXosip2/eXosip.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h> 
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <pthread.h>


#include "cdp/cdp_load.h"
#include "cdp/diameter.h"
#include "cdp/diameter_api.h"
#include "cdp/diameter_ims.h"
#include "cdp/diameter_peer.h"
#include "cdp/globals.h"
#include "cdp/peer.h"
#include "cdp/peerstatemachine.h"
#include "cdp/transaction.h"
#include "cdp/peermanager.h"
#include "cdp/receiver.h"
#include "cdp/tcp_accept.h"
#include "cdp/timer.h"
#include "cdp/utils.h"
#include "cdp/worker.h"

#include "server.h"
#include "main.h"

//int debug = 0;

#endif
