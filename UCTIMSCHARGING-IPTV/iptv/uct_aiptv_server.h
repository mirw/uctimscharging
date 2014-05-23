#ifndef UCT_AIPTV_SERVER_H
#define UCT_AIPTV_SERVER_H

#include "hashtable.h"
#include "../charging.h"

int get_exosip_events();

int iptv_start(char *filename);

void call_terminate(int cid, int did);


#endif
