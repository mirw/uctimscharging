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

#include "../includes.h"
#include "ro.h"

/**
 * Process a Credit Control Request and return the Answer for it.
 * @param ccr - the Diameter CCR
 * @returns the cca Diameter answer
 */
AAAMessage* Ro_CCA( AAAMessage * ccr)
{
	AAAMessage	*cca_msg;
	int ccr_data;
	int result;
	int ccr_avp;
	int onceoff_credit, persec_credit, perMbyte_credit;
	int crdtonce, crdtpsec, crdtpMb;
	char *user_avp;
	str user_avps;

	if (!ccr)
	{
	LOG(L_ERR,"ERROR:Ro_CCA: No message in the request\n");
	return 0;
	}	
	
	LOG(L_DBG,"Ro_CCA - Accounting request received from %s\n",ccr->orig_host->data.s);

	cca_msg	= AAACreateResponse(ccr);
	if (!cca_msg)
	{
	LOG(L_ERR,"ERROR:Ro_CCA: No message in the created response\n");
	return 0;
	}

	if (Ro_get_cc_request_type(ccr, &ccr_data)){//Retrieves accounting event type: START=1, INTERIM=2, STOP=3, EVENT=4
	Ro_get_call_record_id(ccr, &ccr_avp);
	Ro_get_Credit_onceoff(ccr, &crdtonce);
	Ro_get_Credit_persec(ccr, &crdtpsec);
	Ro_get_Credit_perMbyte(ccr, &crdtpMb);
	Ro_get_subscriber(ccr, &user_avp);
	

		switch (ccr_data){
			case START:
				msg_handler(user_avp,ccr_data, &result, ccr_avp, &onceoff_credit, &persec_credit, &perMbyte_credit,0,0,0);
			break;

			case STOP:
				msg_handler(user_avp,ccr_data, &result, ccr_avp, &onceoff_credit, &persec_credit, &perMbyte_credit,crdtonce,crdtpsec,crdtpMb);
			break;

			case INTERIM:
				msg_handler(user_avp,ccr_data, &result, ccr_avp, &onceoff_credit, &persec_credit, &perMbyte_credit,crdtonce,crdtpsec,crdtpMb);
			break;	

			case EVENT:
				msg_handler(user_avp,ccr_data, &result, ccr_avp, &onceoff_credit, &persec_credit, &perMbyte_credit,0,0,0);
			break;

			default:
			return 0;	

			}
	
	}
	
	Ro_add_vendor_specific_appid(cca_msg,IMS_vendor_id_3GPP,IMS_Ro,0 /*IMS_Ro*/);
	
	if (result == 1)
	Ro_add_result_code(cca_msg,DIAMETER_SUCCESS);
	else Ro_add_result_code(cca_msg,0);
	
	if (Ro_get_call_record_id(ccr, &ccr_avp))
	Ro_add_Call_Record_Id(cca_msg, ccr_avp);
	else LOG(L_ERR,"Ro_CCA: Call ID AVP not found\n");
	

	if (Ro_get_subscriber(ccr, &user_avp)){
		user_avps.s = user_avp;
		user_avps.len = strlen(user_avps.s);
		Ro_add_subscriber(cca_msg, user_avps);			
	}else {LOG(L_ERR,"Ro_CCA: Subscriber AVP not found\n");}

	if (Ro_get_cc_request_type(ccr, &ccr_data))
	Ro_add_cc_request_type(ccr, ccr_data);
	else LOG(L_ERR,"Ro_CCA: Request Type AVP not found\n");

	Ro_add_Credit_onceoff(cca_msg, onceoff_credit);
	Ro_add_Credit_persec(cca_msg, persec_credit);
	Ro_add_Credit_perMbyte(cca_msg, perMbyte_credit);
				
	return cca_msg;
}				
 
 
