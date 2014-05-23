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

#ifndef RF_AVP_H
#define RF_AVP_H

#include "../cdp/diameter.h"
#include "../cdp/diameter_api.h"
#include "../cdp/diameter_ims.h"
#include "../cdp/cdp_load.h"
#include "../cdp/utils.h"

#include "../charging.h"

int Rf_add_vendor_specific_appid(AAAMessage *msg,unsigned int vendor_id,
	unsigned int auth_id,unsigned int acct_id);
int Rf_add_Accounting_Record_Type(AAAMessage *msg, unsigned int data);
inline int Rf_get_accounting_record_type(AAAMessage *msg, int *data);
int Rf_add_Accounting_Record_Number(AAAMessage *msg, str data);
int Rf_add_Session_Id(AAAMessage *msg, str data);
int Rf_add_destination_realm(AAAMessage *msg,str data);
int Rf_add_origin_realm(AAAMessage *msg, str data);
int Rf_add_origin_host(AAAMessage *msg, str data);
int Rf_add_destination_host(AAAMessage *msg, str data);

inline int Rf_add_Call_Record_Id(AAAMessage *msg, unsigned int data);
inline int Rf_get_call_record_id(AAAMessage *msg, int *data);
inline int Rf_add_Credit_onceoff(AAAMessage *msg, unsigned int data);
inline int Rf_add_Credit_persec(AAAMessage *msg, unsigned int data);
inline int Rf_add_Credit_perMbyte(AAAMessage *msg, unsigned int data);
inline int Rf_add_subscriber(AAAMessage *msg, str data);
inline int Rf_get_subscriber(AAAMessage *msg, char **data);
AAA_AVP  *Rf_get_avp(AAAMessage *msg,int avp_code,int vendor_id);

#endif 
