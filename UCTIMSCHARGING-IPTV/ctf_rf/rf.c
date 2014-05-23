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
  This file utlizes the Fokus cdp

*/

#include "rf.h"

#include "../cdp/diameter.h"
#include "../cdp/diameter_api.h"
#include "../cdp/diameter_ims.h"
#include "../cdp/cdp_load.h"
#include "../cdp/utils.h"

 
/**
 * Handler for incoming Diameter answers.
 * This is not used as all diameter answers are supposed to be 
 * handled transactionaly
 * @param response - received response
 * @param t - transaction
 * @returns 1
 */
int RfAnserHandler(AAAMessage *response, AAATransaction *t)
{
	switch(response->commandCode){
		default:
			LOG(L_ERR,"ERR: RfAnswerHandler: Unkown Command Code %d\n",
				response->commandCode);
	}
	return 1;
}	

/**
 * Handler for incoming Diameter failures
 * This is not used as all diameter failures are handled transactionaly
 * @param t - transaction
 * @param reason - failure reason
 * @returns 1
 */
int RfFailureHandler(AAATransaction *t,int reason)
{
	LOG(L_INFO,"INF: AAAFailureHandler:  SIP transaction %u %u Reason %d\n",
		t->hash,t->label,reason);
	switch(t->command_code){
		default:
		LOG(L_ERR,"ERR: Rf_FailureHandler: Unkown Command Code %d\n",
			t->command_code);
	}
	return 0;
}

/**
 * Handler for incoming Diameter requests.
 * @param request - the received request
 * @param param - generic pointer
 * @returns the answer to this request
 * but we wont/should not recieve any request here
 */
AAAMessage* RfRequestHandler(AAAMessage *request,void *param)
{
	if (is_req(request)){		
		LOG(L_INFO,"INFO: RfRequestHandler(): We have received a request!!!\n");
		#ifdef WITH_IMS_PM
			ims_pm_diameter_request(request);
		#endif		
		switch(request->applicationId){
        	case IMS_Rf:
				switch(request->commandCode){				
					
					default :
						LOG(L_ERR,"ERR: RfRequestHandler(): - Received unknown request for Rf command %d\n",request->commandCode);
						break;	
				}
				break;
			default:
				LOG(L_ERR,"ERR: RfRequestHandler(): - Received unknown request for app %d command %d\n",
					request->applicationId,
					request->commandCode);
				break;				
		}					
	}
	return 0;		
}
 
 

/**
 * Create and send an Accounting Request
 * This function sends accounting data (CDR) to the CDF
 * @param session_id - ID for the session
 * @param user_subscriber - User name, e.g., Alice
 * @param ohost - the origin host
 * @param orealm - the origin realm
 * @param drealm -  the destination realm
 * @param acc_type - the accounting record type (eg start, interim, stop, event)
 * @param acc_number - the number of the ACR
 * @param dhost - the destination host
 * @param crdtonce - once-off charge value
 * @param crdtpsec - Duration charge value
 * @param crdtpMb - Volume charge value
 * @returns the ACA - accounting answer
 */ 
AAAMessage *Rf_ACR(int session_id, str user_subscriber, str ohost, str orealm, str drealm,
					unsigned int acc_type, str acc_number, str dhost, int crdtonce, int crdtpsec, int crdtpMb)
{
	LOG(L_DBG,"INFO: Rf_ACR() - Preparing accounting request\n");
	AAAMessage *acr=0,*aca=0;
	AAASessionId sessId={0,0};
	AAATransaction *trans=0;
	unsigned int hash=0,label=0;

	
	sessId = AAACreateSession();
	trans = AAACreateTransaction(IMS_Rf,IMS_ACR); 
	
	acr = AAACreateRequest(IMS_Rf,IMS_ACR,Flag_Proxyable,&sessId);
	if (!acr) goto error;
	if (!Rf_add_vendor_specific_appid(acr,IMS_vendor_id_3GPP,IMS_Rf,0 /*IMS_Rf*/)) goto error;
	if (!Rf_add_origin_realm(acr, orealm)) goto error;
	if (!Rf_add_destination_realm(acr, drealm)) goto error;
	if (!Rf_add_Accounting_Record_Type(acr, acc_type)) goto error;
	if (!Rf_add_Accounting_Record_Number(acr, acc_number)) goto error;
	if (!Rf_add_Call_Record_Id(acr, session_id)) goto error;
	if (!Rf_add_subscriber(acr, user_subscriber)) goto error;
	if (!Rf_add_Credit_onceoff(acr, crdtonce)) goto error;
	if (!Rf_add_Credit_persec(acr, crdtpsec)) goto error;	
	if (!Rf_add_Credit_perMbyte(acr, crdtpMb)) goto error;
	if (!Rf_add_destination_host(acr, dhost)) goto error;
	if (!Rf_add_origin_host(acr, ohost)) goto error;
		
	trans->hash=hash;
	trans->label=label;
		
	LOG(L_INFO,"INFO: sending Diameter Accounting Request to: %s\n",acr->dest_host->data.s);
	aca = AAASendRecvMessageToPeer(acr,&dhost); //send a charging request or answer
	        
	if(!aca)
	{
		LOG(L_ERR,"ERROR: Rf_ACR - No message in the accounting reply\n");
		return 0;
	}
	AAADropSession(&sessId);
	AAADropTransaction(trans);
	return aca;
	
error: 	
	if (trans) AAADropTransaction(trans);
	if (sessId.s) AAADropSession(&sessId);
	if (acr) AAAFreeMessage(&acr);
	return 0;	
		
}	
