/*
  The UCT IMS IPTV Application Server
  Copyright (C) 2008 - University of Cape Town
  Robert Marston <rmarston@crg.ee.uct.ac.za>
  David Waiting <david@crg.ee.uct.ac.za>

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

/*FIXME: Dave took these from the client probably don't need a few of them */
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <regex.h>
#include <sys/time.h>
#include <eXosip2/eXosip.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include "hashtable.h"
#include "hashtable_itr.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

#define PORT 8010

#include "includes.h"
#include "uct_aiptv_server.h"


int charge_type;

/* Set up functions and variables for the hash table: Can be removed to reduce runtime and code size costs 
*  See hashtable.h for more information */
struct key
{
   unsigned char *sip_uri;
};

struct key1
{
   char *sip_uri;
};

struct value
{
   unsigned char *rtsp_uri;
};

struct value1
{
   char *rtsp_uri;
};

DEFINE_HASHTABLE_INSERT(insert_some, struct key, struct value);
DEFINE_HASHTABLE_SEARCH(search_some, struct key1, struct value1);
DEFINE_HASHTABLE_REMOVE(remove_some, struct key, struct value);
DEFINE_HASHTABLE_ITERATOR_SEARCH(search_itr_some, struct key);

/* Function used for converting of key to hash value */
static unsigned int
hashfromkey(void *ky)
{
    int i;
    unsigned int hashValue = 0;
    struct key1 *k = (struct key1 *)ky;
    for(i = 0; i < strlen(k->sip_uri); i++) {
        hashValue+= k->sip_uri[i] - 0;
    }
    return hashValue;
}

/* Function used to compare if two keys are equal */
static int
equalkeys(void *k1, void *k2)
{
    struct key1 *key2 = (struct key1 *)k1;
    struct key1 *key3 = (struct key1 *)k2;
    return (0 == strcmp(key2->sip_uri,key3->sip_uri));
}

/*******************************************************************************************/

int runcond=1;
struct hashtable *hashTable;

/* Function: Catches the ctrl + c quiet event */
void stophandler(int signum) {
    LOG(L_INFO,"Shutting down...\n");
    runcond=0;
}

/* Function: Stores the key-value pair read from a file in the hash table */
int hashKeyVal(xmlNode *a_node, struct hashtable *hashTable) {
    struct key *k;
    struct value *v;
    xmlNode *cur_node;

    for(cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if(!xmlStrcmp(cur_node->name, (const xmlChar *)"key")) {
                k = (struct key *)malloc(sizeof(struct key));
                if (k == NULL) {
                    printf("hashkeyval(): Ran out of memory allocating a key\n");
                    return 1;
                }
                k->sip_uri = xmlNodeGetContent(cur_node);
        }
        else if(!xmlStrcmp(cur_node->name, (const xmlChar *)"value")) {
                v = (struct value *)malloc(sizeof(struct value));
                v->rtsp_uri = xmlNodeGetContent(cur_node);
                /* Try insert key and value into table else return and error */
                if (!insert_some(hashTable,k,v)) {
                    printf("Error: hashkeyval - Could not insert key-value pair into hash table\n");
                    return 1;
                }
        }
    }
return 0;
}

/* Function: Uses libxml to read from a file and extract the key-value pairs that will be stored in the hash table */
int read_file_fill_table(struct hashtable *hashTable, char *filename) {
   // FILE *kvfile;
    xmlDocPtr doc;
    xmlNodePtr cur_node;
    int kvCount;        /* Used to keep track of how many key-value pairs were found */

    LOG(L_INFO,"Populating table with key-value pairs...\n");

    doc = xmlParseFile(filename);

    if (doc == NULL ) {
        fprintf(stderr,"ERROR: XML key-value file not parsed successfully\n");
        return 1;
    }

    cur_node = xmlDocGetRootElement(doc);
    if (cur_node == NULL) {
        fprintf(stderr,"ERROR: key-value file is empty\n");
        xmlFreeDoc(doc);
        return 1;
    }

    if (xmlStrcmp(cur_node->name, (const xmlChar *) "key-value_pairs")) {
        fprintf(stderr,"ERROR: document of the wrong type, root node != key-value_pairs\n");
        xmlFreeDoc(doc);
        return 1;
    }

    cur_node = cur_node->xmlChildrenNode;
    kvCount = 0;
    while(cur_node != NULL) {
        if ((!xmlStrcmp(cur_node->name, (const xmlChar *)"key-value_pair"))) {
            kvCount++;
            /*Find key and value and then add to hash table */
            hashKeyVal(cur_node->children,hashTable);
        }
        /* Assign current node pointer to pointer of next node */
        cur_node = cur_node->next;
    }
    LOG(L_INFO,"Number of key-value pairs found in file %s is %d\n",filename, kvCount);
    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();
    
    printf("Done.\n");
    return 0;
}


/*Function: Used to search the hashtable for a given key and extract the value if the key is found */
char* hash_lookupValue(struct hashtable *hashTable, char* sip_uri) {
    struct key1 *k;
    struct value1 *found;
    char *rtsp_uri;

    k = (struct key1 *)malloc(sizeof(struct key1));
    if (k == NULL) {
        printf("ran out of memory allocating a key\n");
        return "serv_err";
    }

    k->sip_uri = sip_uri;
    if ((found = search_some(hashTable,k)) == NULL) {
        printf("Error: key not found\n");
        return "key_err";
    }

    rtsp_uri = found->rtsp_uri;
    free(k);
    return rtsp_uri;
}

/*Function: Main eXosip function to get an event and take action on it*/
int get_exosip_events() {
    eXosip_event_t *je;

    if((je = eXosip_event_wait (0, 50)) != NULL) {
        LOG(L_INFO,"Event type: %d %s\n", je->type, je->textinfo);
	//LOG(L_INFO,"The Call ID is %d\n",je->cid);

        eXosip_lock ();
        eXosip_automatic_action();
        eXosip_unlock ();

        /*If we recieved an invite from the client*/
        if (je->type == EXOSIP_CALL_INVITE) {
            char *requested_content, *rtsp_redirect_uri, *subscriber, *sub_domain, *call_id_number;
            osip_message_t *answer;
            
            //Get CONTACT URI from INVITE messages
			char *contact;
			osip_contact_t *contact_header;
			osip_message_get_contact((osip_message_t *)je->request,0, &contact_header);
			if(contact_header)
			{
				osip_contact_to_str(contact_header, &contact);
			}
	
            /* The unique identifer for the content is placed into the address of the invite so we extract it here */
            requested_content = (((je->request)->to)->url)->username;
	    	
            /* Here we use the identifer as the key and look up the approriate rstp address in the hash table */
            rtsp_redirect_uri = hash_lookupValue(hashTable, (char *)requested_content);

            /* Check that all went ok in finding the value for the given key */
            eXosip_lock ();
            if(!strcmp(rtsp_redirect_uri, "serv_err")) {
                /* If there was a problem allocating memory then return message to client indicating server error */
                eXosip_call_build_answer(je->tid, 500, &answer);
                eXosip_call_send_answer(je->tid, 500, answer);
            }
            else if(!strcmp(rtsp_redirect_uri, "key_err")) {
                /* If we couldn't find the key in the hash table then return a message to the client indicating this*/
                eXosip_call_build_answer(je->tid, 400, &answer);
                eXosip_call_send_answer(je->tid, 400, answer);
            }
            else {
                /* Send 200 OK to client with rtsp address appended in  the content type header field */
                eXosip_call_build_answer(je->tid, 200, &answer);
                eXosip_unlock ();

                char content_type_header[500];
                sprintf(content_type_header, "message/external-body; access-type=\"URL\"; "
                "expiration=\"Sat, 01 January 2011 09:00:00 GMT\"; URL=\"%s\"", rtsp_redirect_uri);

                if(osip_message_set_content_type(answer, content_type_header) != 0)
                LOG(L_ERR,"ERROR: get_exosip_events() - setting content type\n");
		
		eXosip_lock();
		
		/* Charging Stuff come here */
		subscriber = (((je->request)->from)->url)->username;
	    sub_domain = (((je->request)->from)->url)->host;
	    call_id_number =  ((je->request)->call_id)->number;	
        LOG(L_DBG,"Client request: %s, callId %s located at %s Domain %s is requesting %s\n",subscriber, call_id_number,contact, sub_domain,requested_content);
            	
		charge_type = get_charge_type_from_sdp(je->request);
		if(charge_type != 0){
			//transformations
			if(charge_type == 2) charge_type=0;
				
			if(charge_iptv(charge_type,START,je->cid,je->request->from->url->username,je->did))//Derive charge type from user profile
				{
					LOG(L_INFO,"UCT-CHARGE:get_exosip_events - Sending RTSP address to client\n");
					//Tracking purposes
					aiptv_log_message(answer);
					eXosip_call_send_answer(je->tid, 200, answer);       //send IPTV response to IMS client
				}
				else
				{
					LOG(L_ERR,"ERROR: get_exosip_events() - UCT-CHARGE - Something wrong with the charging system\n");
				}
			}
		}
            eXosip_unlock ();
            eXosip_event_free(je);
        }
               
	if (je->type == EXOSIP_CALL_CLOSED ) {
		
		/*Charging stuff comes here  */
		LOG(L_INFO,"Stop Charging requested\n");
		charge_type = get_charge_type_from_a_client(je->cid);
		if(charge_type != 0){
			//transformations
			if(charge_type == 2) charge_type=0;
		
			if(charge_iptv(charge_type,STOP,je->cid,je->request->from->url->username,je->did) == 1)//Derive charge type from user profile
				{
					LOG(L_INFO,"UCT-CHARGE:get_exosip_events - Accounting terminated\n");
				}
			else
				{
					LOG(L_ERR,"ERROR: get_exosip_events() - UCT-CHARGE - Something wrong with the charging system\n");
				}
			}
		
     }
    }

    return 0;
}

/*
 * Starts the UCT Advanced IPTV Server
 @Params
 char *filename - the IPTV hash table configuration file
 * 
 * */

int iptv_start(char *filename)
{
	LOG(L_INFO,"UCT Media Control Function\n");
	LOG(L_INFO,"Developers: David Waiting and Robert Marston (2008)\n");    
	    
    eXosip_init();

    if(eXosip_listen_addr(IPPROTO_UDP, NULL, PORT, AF_INET, 0) != 0) {
        LOG(L_ERR,"Error: Port busy - exiting\n");
        return -1;
    }
    else {
        LOG(L_INFO,"eXosip started and listening on port = %d\n", PORT);
    }
    LOG(L_INFO,"Creating Hashtable...\n");
    /* Create the hash table for storing content identifers and corresponding rtsp address */
    hashTable = create_hashtable(16, hashfromkey, equalkeys);
    /* Exit if hash table could not be created */
    if (NULL == hashTable) exit(-1);

    /* Read in the key-value pairs from file for hash table */
    if(!read_file_fill_table(hashTable, filename)) {
       
        /* If success then start listening for client invites */
        LOG(L_INFO,"IPTV Server is ready to accept client requests... \n");
     }
    return 1;

}
/*
 * Terminate call on exhausted credits - used for online charging
 * cid - call id je->cid
 * did - dialog id je->did
 */
void call_terminate(int cid, int did)
{
	eXosip_call_terminate(cid, did);
}
