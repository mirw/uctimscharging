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
  * 
  This file utilizes the Fokus cdp

*/
#include "ro_avp.h"  

/**
 * Returns the value of a certain AVP from a Diameter message.
 * westart searching from the beginning 0
 * @param msg - Diameter message to look into
 * @param avp_code - the code to search for
 * @param vendorid - the value of the vendor id to look for or 0 if none
 * * @returns the AAA_AVP* if found, NULL if not 
 */
AAA_AVP  *Ro_get_avp(AAAMessage *msg,int avp_code,int vendor_id)
{
	AAA_AVP *avp;
	/* param checking */
	if (!msg) {
		LOG(L_ERR,"ERR:Ro_get_avp: param msg passed null !!\n");
		return 0;
	}
	
	avp =  AAAFindMatchingAVP(msg,0,avp_code,vendor_id,0);
	if (avp==0){
		LOG(L_ERR,"ERR: Ro_get_avp: Failed finding avp\n");
		return 0;
	}
	else 
	return avp;
}


/**
 * Create and add an AVP to a Diameter message.
 * @param m - Diameter message to add to 
 * @param d - the payload data
 * @param len - length of the payload data
 * @param avp_code - the code of the AVP
 * @param flags - flags for the AVP
 * @param vendorid - the value of the vendor id or 0 if none
 * @param data_do - what to do with the data when done
 * @param func - the name of the calling function, for debugging purposes
 * @returns 1 on success or 0 on failure
 */
static inline int Ro_add_avp(AAAMessage *m,char *d,int len,int avp_code,
	int flags,int vendorid,int data_do,const char *func)
{
	AAA_AVP *avp;
	if (vendorid!=0) flags |= AAA_AVP_FLAG_VENDOR_SPECIFIC;
	avp = AAACreateAVP(avp_code,flags,vendorid,d,len,data_do);
	if (!avp) {
		LOG(L_ERR,"ERR: Failed creating avp\n");
		return 0;
	}
	if (AAAAddAVPToMessage(m,avp,m->avpList.tail)!=AAA_ERR_SUCCESS) {
		LOG(L_ERR,"ERR: Failed adding avp to message\n");
		AAAFreeAVP(&avp);
		return 0;
	}
	return 1;
}

/**
 * Creates and adds a Session ID AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Ro_add_session_id(AAAMessage *msg, str data)
{
	return 
	Ro_add_avp(msg,data.s,data.len,
		AVP_Session_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a Origin Host AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Ro_add_origin_host(AAAMessage *msg, str data)
{
	return
	Ro_add_avp(msg,data.s,data.len,
		AVP_Origin_Host,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a Origin Realm AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Ro_add_origin_realm(AAAMessage *msg, str data)
{
	return
	Ro_add_avp(msg,data.s,data.len,
		AVP_Origin_Realm,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a Authorization Application ID AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Ro_add_auth_application_id(AAAMessage *msg, str data)
{
	return 
	Ro_add_avp(msg,data.s,data.len,
	AVP_Auth_Application_Id,
	AAA_AVP_FLAG_MANDATORY,
	0,
	AVP_DUPLICATE_DATA,
	__FUNCTION__);
}

/**
 * Creates and adds a Service Context ID AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Ro_add_service_context_id(AAAMessage *msg, str data)
{
	return
	Ro_add_avp(msg,data.s,data.len,
		AVP_Service_context_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
}

/**
 * Creates and adds a Credit Control Request Type AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Ro_add_cc_request_type(AAAMessage *msg, unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Ro_add_avp(msg,x,4,
		AVP_CC_Request_Type,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
		
}

/**
 * Returns theAVP_Accounting_Record_Type AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns the AVP payload on success or an empty string on error
 */
inline int Ro_get_cc_request_type(AAAMessage *msg, int *data)
{
	AAA_AVP *avp;
	avp = Ro_get_avp(msg,
		AVP_CC_Request_Type,0);
	if (!avp)
	{
	LOG(L_ERR,"ERR:Ro_get_cc_request_type: AVP_CC_Request_type not found\n");
	return 0;
	}
	*data = get_4bytes(avp->data.s);
	return 1;
}


/**
 * Creates and adds a Credit Control Request Number AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Ro_add_cc_request_number(AAAMessage *msg, str data)
{
	return	
	Ro_add_avp(msg,data.s,data.len,
	AVP_CC_Request_Number,
	AAA_AVP_FLAG_MANDATORY,
	0,
	AVP_DUPLICATE_DATA,
	__FUNCTION__);
}

/**
 * Creates and adds a Destination Host AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Ro_add_destination_host(AAAMessage *msg, str data)
{
	return
	Ro_add_avp(msg,data.s,data.len,
		AVP_Destination_Host,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
}


/**
 * Creates and adds a Call Record ID AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */

inline int Ro_add_Call_Record_Id(AAAMessage *msg, unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Ro_add_avp(msg,x,4,
		AVP_Call_Record_Id,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
}

/**
 * Returns the AVP_Call_Record_Id AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns 1 on success or 0 on error
 */
inline int Ro_get_call_record_id(AAAMessage *msg, int *data)
{
	AAA_AVP *avp;
	//str s;
	avp = Ro_get_avp(msg,AVP_Call_Record_Id,0);
	if (!avp)
	{
	LOG(L_ERR,"ERR:Ro_get_call_record_id: AVP_Call_Record_Id not found\n");
	return 0;
	}
	*data = get_4bytes(avp->data.s); //Retrieves the value for the accounting type from the AVP
	return 1;

}

/**
 * Creates and adds a onceoff_credit AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Ro_add_Credit_onceoff(AAAMessage *msg, unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Ro_add_avp(msg,x,4,
		AVP_Credit_onceoff,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
}

/**
 * Creates and adds a persec_credit AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Ro_add_Credit_persec(AAAMessage *msg, unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Ro_add_avp(msg,x,4,
		AVP_Credit_persec,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
}

/**
 * Creates and adds a perMbyte_credit AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */
inline int Ro_add_Credit_perMbyte(AAAMessage *msg, unsigned int data)
{
	char x[4];
	set_4bytes(x,data);
	return 
	Ro_add_avp(msg,x,4,
		AVP_Credit_perMbyte,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
}

/**
 * Returns the AVP_Credit_onceoff AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns 1 on success or 0 on error
 */
inline int Ro_get_Credit_onceoff(AAAMessage *msg, int *data)
{
	AAA_AVP *avp;
	//str s;
	avp = Ro_get_avp(msg,AVP_Credit_onceoff,0);
	if (!avp)
	{
	LOG(L_ERR,"ERR:Ro_get_Credit_onceoff: AVP_Credit_onceoff not found\n");
	return 0;
	}
	*data = get_4bytes(avp->data.s); 
	return 1;
}

/**
 * Returns the AVP_Credit_persec AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns 1 on success or 0 on error
 */
inline int Ro_get_Credit_persec(AAAMessage *msg, int *data)
{
	AAA_AVP *avp;
	//str s;
	avp = Ro_get_avp(msg,AVP_Credit_persec,0);
	if (!avp)
	{
	LOG(L_ERR,"ERR:Ro_get_Credit_onceoff: AVP_Credit_persec not found\n");
	return 0;
	}
	*data = get_4bytes(avp->data.s); 
	return 1;
}

/**
 * Returns the AVP_Credit_perMbyte AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns 1 on success or 0 on error
 */
inline int Ro_get_Credit_perMbyte(AAAMessage *msg, int *data)
{
	AAA_AVP *avp;
	//str s;
	avp = Ro_get_avp(msg,AVP_Credit_perMbyte,0);
	if (!avp)
	{
	LOG(L_ERR,"ERR:Ro_get_Credit_perMbyte: AVP_Credit_perMbyte not found\n");
	return 0;
	}
	*data = get_4bytes(avp->data.s); 
	return 1;
}


/**
 * Adds a subscriber name AVP.
 * @param msg - the Diameter message to add to.
 * @param data - the value for the AVP payload
 * @returns 1 on success or 0 on error
 */

inline int Ro_add_subscriber(AAAMessage *msg, str data)
{
	return
	Ro_add_avp(msg,data.s,data.len,
		AVP_Subscriber,
		AAA_AVP_FLAG_MANDATORY,
		0,
		AVP_DUPLICATE_DATA,
		__FUNCTION__);
	
}


/**
 * Returns the AVP_Subscriber AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns 1 on success or 0 on error
 */
inline int Ro_get_subscriber(AAAMessage *msg, char **data)
{
	AAA_AVP *avp;
	//str s;
	avp = Ro_get_avp(msg,AVP_Subscriber,0);
	if (!avp)
	{
	LOG(L_ERR,"ERR:Ro_get_subscriber: AVP_Subscriber not found\n");
	return 0;
	}
	*data = avp->data.s; 
	return 1;
}





