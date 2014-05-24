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

#include "../includes.h"
#include "rf.h"

/**
 * Process an Accounting Request and return the Answer for it.
 * @param acr - the Diameter accounting request - received from a CTF
 * @returns  - AAAMessage* - the Diameter Accounting answer or NULL on error
 */
AAAMessage* Rf_ACA( AAAMessage * acr)
{
	AAAMessage	*aca_msg;
	int acr_data;
	int result;
	int acr_avp;
	int onceoff_credit, persec_credit, perMbyte_credit;	
	int crdtonce, crdtpsec, crdtpMb;
	char *user_avp;
	str user_avps;
				
	if (!acr)
	{
	LOG(L_ERR,"ERROR:Rf_ACA: No message in the request\n");
	return 0;
	}

	LOG(L_INFO,"UCT-CHARGE:Rf_ACA - Accounting request received from %s\n",acr->orig_host->data.s);

	aca_msg	=  AAACreateResponse(acr);
	if (!aca_msg)
	{
	LOG(L_ERR,"ERROR:Rf_ACA: No message in the created response\n");
	return 0;
	}
	
	if (Rf_get_accounting_record_type(acr, &acr_data)){ //Retrieves accounting event type: START=1, INTERIM=2, STOP=3, EVENT=4
		Rf_get_accounting_record_number(acr, &acr_avp);
		Rf_get_user(acr, &user_avp);

		switch (acr_data){
			case START:
				msg_handler(user_avp,acr_data, &result, acr_avp,&onceoff_credit, &persec_credit, &perMbyte_credit,0,0,0);//Trigger online charging
			break;

			case STOP:
				msg_handler(user_avp,acr_data,&result,acr_avp,&onceoff_credit, &persec_credit, &perMbyte_credit,crdtonce,crdtpsec,crdtpMb);//Trigger online charging
			break;

			case INTERIM:
				msg_handler(user_avp,acr_data, &result, acr_avp,&onceoff_credit, &persec_credit, &perMbyte_credit,crdtonce,crdtpsec,crdtpMb);//Trigger online charging
			break;	

			case EVENT:
				msg_handler(user_avp,acr_data, &result, acr_avp,&onceoff_credit, &persec_credit, &perMbyte_credit,0,0,0);//Trigger online charging
			break;

			default:
			return 0;	

			}
	}
		
	Rf_add_vendor_specific_appid(aca_msg,IMS_vendor_id_3GPP,IMS_Rf,0 /*IMS_Rf*/);
	Rf_add_result_code(aca_msg,DIAMETER_SUCCESS);

	if (Rf_get_accounting_record_type(acr, &acr_data)){
		Rf_add_Accounting_Record_Type(acr, acr_data);
		LOG(L_DBG,"Rf_ACA: Adding accounting record type %d\n",acr_data);			
	}else {LOG(L_ERR,"Rf_ACA: Accounting record Type AVP not found\n");}

	if (Rf_get_accounting_record_number(acr, &acr_avp))
	Rf_add_Call_Record_Id(aca_msg, acr_avp);

	if (Rf_get_user(acr, &user_avp)){
		user_avps.s = user_avp;
		user_avps.len = strlen(user_avps.s);
		Rf_add_subscriber(aca_msg, user_avps);	
		LOG(L_DBG,"Rf_ACA: Adding User-Name\n");
	}else {LOG(L_ERR,"Rf_ACA: User-Name AVP not found\n");}

	Rf_add_Credit_onceoff(aca_msg, onceoff_credit);
	Rf_add_Credit_persec(aca_msg, persec_credit);
	Rf_add_Credit_perMbyte(aca_msg, perMbyte_credit);
					
	return aca_msg;			
}				
