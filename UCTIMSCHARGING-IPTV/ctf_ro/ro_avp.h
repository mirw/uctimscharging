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
  
#ifndef RO_AVP_H
#define RO_AVP_H

/** AVPs for the as Credit control Ro functions (CCRs)*/

#include "../cdp/diameter.h"
#include "../cdp/diameter_api.h"
#include "../cdp/diameter_ims.h"
#include "../cdp/cdp_load.h"
#include "../cdp/utils.h"

int Ro_add_session_id(AAAMessage *msg, str data);
int Ro_add_origin_host(AAAMessage *msg, str data);
int Ro_add_origin_realm(AAAMessage *msg, str data);
int Ro_add_auth_application_id(AAAMessage *msg, str data);
int Ro_add_service_context_id(AAAMessage *msg, str data);
int Ro_add_cc_request_type(AAAMessage *msg, unsigned int data);
int Ro_add_cc_request_number(AAAMessage *msg, str data);
int Ro_add_destination_host(AAAMessage *msg, str data);
AAA_AVP  *Ro_get_avp(AAAMessage *msg,int avp_code,int vendor_id);

inline int Ro_add_Call_Record_Id(AAAMessage *msg, unsigned int data);
inline int Ro_get_call_record_id(AAAMessage *msg, int *data);
inline int Ro_get_Credit_onceoff(AAAMessage *msg, int *data);
inline int Ro_get_Credit_persec(AAAMessage *msg, int *data);
inline int Ro_get_Credit_perMbyte(AAAMessage *msg, int *data);
inline int Ro_add_Credit_onceoff(AAAMessage *msg, unsigned int data);
inline int Ro_add_Credit_persec(AAAMessage *msg, unsigned int data);
inline int Ro_add_Credit_perMbyte(AAAMessage *msg, unsigned int data);

inline int Ro_add_subscriber(AAAMessage *msg, str data);
inline int Ro_get_subscriber(AAAMessage *msg, char **data);
inline int Ro_get_cc_request_type(AAAMessage *msg, int *data);


#endif
