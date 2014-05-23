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

#include "server.h"

//#define YES  50
//#define NO   51


/**
 * Returns the value of a certain AVP from a Diameter message.
 * @param m - Diameter message to look into
 * @param avp_code - the code to search for
 * @param vendorid - the value of the vendor id to look for or 0 if none
 * @param func - the name of the calling function, for debugging purposes
 * @returns the str with the payload on success or an empty string on failure
 */
static inline str get_avp(AAAMessage *msg,int avp_code,int vendor_id,
							const char *func)
{
	AAA_AVP *avp;
	str r={0,0};
	
	avp = AAAFindMatchingAVP(msg,0,avp_code,vendor_id,0);
	if (avp==0){
		LOG(L_INFO,"INFO: Failed finding avp\n");
		return r;
	}
	else 
		return avp->data;
}


/**
 * Returns the Result-Code AVP from a Diameter message.
 * @param msg - the Diameter message
 * @returns 1 on success or 0 on error
 */
inline int get_result_code(AAAMessage *msg, int *data)
{
	str s;
	s = get_avp(msg,
		AVP_Result_Code,
		0,
		__FUNCTION__);
	if (!s.s) return 0;
	*data = get_4bytes(s.s);
	return 1;
}

AAAMessage *send_unknown_request_answer(AAAMessage *req)
{
        AAAMessage *ans=0;
        char x[4];
        AAA_AVP *avp;


        /* UAA header is created based on the UAR */
        ans = AAANewMessage(req->commandCode,req->applicationId,0,req);

        if (!ans) return 0;

        set_4bytes(x,(unsigned int) DIAMETER_UNABLE_TO_COMPLY);

        avp = AAACreateAVP(AVP_Result_Code,AAA_AVP_FLAG_MANDATORY,0,x,4,AVP_DUPLICATE_DATA);
        if (!avp) {
                LOG(L_ERR,"ERR: Failed creating avp for result code\n");
                AAAFreeMessage(&ans);
                return 0;
        }
        if (AAAAddAVPToMessage(ans,avp,ans->avpList.tail)!=AAA_ERR_SUCCESS) {
                LOG(L_ERR,"ERR: Failed adding avp to message\n");
                AAAFreeAVP(&avp);
                AAAFreeMessage(&ans);
                return 0;
        }
        return ans;
}

/*
 * Receives and processes incoming Diameter Messages
 @Params
 peer *p - The Diameter peer from which the message is received
 AAAMessage *msg - The Diameter message
 void* ptr - a pointer to the function to be executed upon exit
 ** 
 * */

int process_incoming(peer *p,AAAMessage *msg,void* ptr)
{
	AAAMessage *ans=0;
	int result;
	int crdtonce, crdtpsec, crdtpMb;
	char *user_subscriber;
	int acr_cid;
	
	LOG(L_INFO,"process_incoming - Received Diameter message from %s\n",msg->orig_host->data.s);
	
	switch(msg->applicationId){
		case IMS_Rf:
			LOG(L_DBG,"process_incoming IMS_RF - Received Diameter Accounting Answer from %s\n",msg->orig_host->data.s);
			if(get_result_code(msg,&result))
			{	
				if(result == DIAMETER_SUCCESS)
				{
					if(!Rf_get_call_record_id(msg, &acr_cid)) return 0;
					if(!Rf_get_subscriber(msg, &user_subscriber)) return 0;
					if(!Ro_get_Credit_onceoff(msg, &crdtonce)) return 0;
					if(!Ro_get_Credit_persec(msg, &crdtpsec)) return 0;
					if(!Ro_get_Credit_perMbyte(msg, &crdtpMb)) return 0;
					event_handler(YES,0,acr_cid,user_subscriber,crdtonce,crdtpsec,crdtpMb);
				}
				else
				{
					LOG(L_ERR,"process_incoming IMS_RF- Diameter returned no Success on ACA");
					return 0;
				}
			}
			else
			{
				LOG(L_ERR,"process_incoming IMS_RF - Result code not found\n");	
				return 0;		
			}	
			return 1;
		break;
		
		case IMS_Ro:
			LOG(L_DBG,"process_incoming IMS_Ro- Received Diameter Credit Control Answer from %s\n",msg->orig_host->data.s);
			if(get_result_code(msg,&result))
			{
				if(result == DIAMETER_SUCCESS)
				{
					if(!Ro_get_call_record_id(msg, &acr_cid)) return 0;
					if(!Ro_get_subscriber(msg, &user_subscriber)) return 0;
					if(!Ro_get_Credit_onceoff(msg, &crdtonce)) return 0;
					if(!Ro_get_Credit_persec(msg, &crdtpsec)) return 0;
					if(!Ro_get_Credit_perMbyte(msg, &crdtpMb)) return 0;
					event_handler(YES,1,acr_cid,user_subscriber,crdtonce,crdtpsec,crdtpMb);
				}
				else
				{
					LOG(L_ERR,"process_incoming IMS_Ro - Diameter returned no Success on CCA");
					return 0;
				}
			}
			else 
			{
				LOG(L_ERR,"process_incoming IMS_Ro - Result code not found\n");
				return 0;
			}
			return 1;
		break;
		default:
	        LOG(L_ERR,"process_incoming IMS_Ro: Received unserviced AppID [%d]\n",msg->applicationId);
            ans = send_unknown_request_answer(msg);               	
	}
	return 1;
}
