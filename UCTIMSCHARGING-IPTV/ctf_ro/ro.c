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
#include "ro.h"
 
/**
 * Handler for incoming Diameter answers.
 * This is not used as all diameter answers are handled transactionaly
 * @param response - received response
 * @param t - transaction
 * @returns 1
 */
int RoAnserHandler(AAAMessage *response, AAATransaction *t)
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
int RoFailureHandler(AAATransaction *t,int reason)
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
AAAMessage* RoRequestHandler(AAAMessage *request,void *param)
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
 * Create and send an Credit Control Request and returns the Answer received for it.
 * This function credit control messages to the OCS
 * @param session_id - the ID number for the session
 * @param user_subscriber - user name, e.g. Alice
 * @param ohost - the origin host
 * @param orealm - the origin realm
 * @param auth_app_id - the authorization application ID
 * @param service_context_id - the service context ID, to identify type of service
 * @param cc_type -  the Credit Control Type (e.g. START, INTERIM, STOP)
 * @param cc_number - the Credit Control Number
 * @param dhost - the destination host
 * @param crdtonce - once-off charge value
 * @param crdtpsec - Duration charge value
 * @param crdtpMb - Volume charge value
 * @returns the CCA
 */ 
AAAMessage *Ro_CCR(int session_id, str user_subscriber, str ohost, str orealm, str auth_app_id,
			str service_context_id, unsigned int cc_type, str cc_number, str dhost, int crdtonce, int crdtpsec, int crdtpMb)
{
	LOG(L_DBG,"DBG: Ro_CCR() - Preparing accounting request for %s\n",user_subscriber.s);	
	AAAMessage *ccr=0,*cca=0;
	AAASessionId sessId={0,0};
	AAATransaction *trans=0;
	unsigned int hash=0,label=0;
	//char buf[200];

	sessId = AAACreateSession();
	trans=AAACreateTransaction(IMS_Ro,IMS_CCR);
	
	ccr = AAACreateRequest(IMS_Ro,IMS_CCR,Flag_Proxyable,&sessId);
	if (!ccr) goto error;

	if (!Ro_add_origin_realm(ccr, orealm)) goto error;
	if (!Ro_add_cc_request_type(ccr, cc_type)) goto error;
	if (!Ro_add_cc_request_number(ccr, cc_number)) goto error;
	if (!Ro_add_auth_application_id(ccr, auth_app_id)) goto error;
	if (!Ro_add_service_context_id(ccr, service_context_id)) goto error;

	if (!Ro_add_subscriber(ccr, user_subscriber)) goto error;	
	if (!Ro_add_Call_Record_Id(ccr, session_id)) goto error;
	if (!Ro_add_Credit_onceoff(ccr, crdtonce)) goto error;
	if (!Ro_add_Credit_persec(ccr, crdtpsec)) goto error;	
	if (!Ro_add_Credit_perMbyte(ccr, crdtpMb)) goto error;

	if (!Ro_add_destination_host(ccr, dhost)) goto error;
	if (!Ro_add_origin_host(ccr, ohost)) goto error;
		
	trans->hash=hash;
	trans->label=label;
	
	LOG(L_INFO,"INFO: sending Diameter Credit Control Request to: %s\n",ccr->dest_host->data.s);
	cca = AAASendRecvMessageToPeer(ccr,&dhost);

	if(!cca)
	{
		LOG(L_ERR,"Error: Ro_CCR - No message in the accounting reply\n");
		return 0;
	}
				
	AAADropSession(&sessId);
	AAADropTransaction(trans);
	
	return cca;
	
error:
	if (trans) AAADropTransaction(trans);
	if (sessId.s) AAADropSession(&sessId);
	if (ccr) AAAFreeMessage(&ccr);
	return 0;	
}	
