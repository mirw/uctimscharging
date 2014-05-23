/*
 * $id$ config.h $date $author$ Dragos Vingarzan dvi vingarzan@gmail.com
 *
 * Copyright (C) 2005 Fhg Fokus
 *
 */


//#ifndef _c_diameter_peer_utils_config_h
//#define _c_diameter_peer_utils_config_h
#ifndef config_h
#define config_h


//#include "../globals.h"
//#include "../includes.h"

extern unsigned long shm_mem_size;
extern int memlog;


#define SHM_MEM_SIZE 16*1024*1024 /* 1 MB */

#define PKG_MEM_POOL_SIZE 1024*1024 /* 1 MB */

/*used only if PKG_MALLOC is defined*/
//#define PKG_MEM_POOL_SIZE 8*1024*1024

/*used if SH_MEM is defined*/
//#define SHM_MEM_SIZE 32


/* maximum number of addresses on which we will listen */
#define MAX_LISTEN 16

/* default number of child processes started */
#define CHILD_NO    8

#define RT_NO 2 /* routing tables number */
#define FAILURE_RT_NO RT_NO /* on_failure routing tables number */
#define ONREPLY_RT_NO RT_NO /* on_reply routing tables number */
#define BRANCH_RT_NO RT_NO /* branch_route routing tables number */
#define ONSEND_RT_NO 1  /* onsend_route routing tables number */
#define DEFAULT_RT 0 /* default routing table */

#define MAX_REC_LEV 100 /* maximum number of recursive calls */
#define ROUTE_MAX_REC_LEV 100 /* maximum number of recursive calls
							   for route()*/
#define MAX_URI_SIZE 1024	/* used when rewriting URIs */

//#define CONTENT_LENGTH "Content-Length: "
//#define CONTENT_LENGTH_LEN (sizeof(CONTENT_LENGTH)-1)

#define RECEIVED        ";received="
#define RECEIVED_LEN (sizeof(RECEIVED) - 1)

/* max length of the text of fifo 'print' command */
#define MAX_PRINT_TEXT 256


#endif

