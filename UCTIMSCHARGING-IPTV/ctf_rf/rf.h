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

#ifndef AS_RF_H
#define AS_RF_H

#include "../cdp/diameter.h"
#include "../cdp/diameter_api.h"
#include "../cdp/diameter_ims.h"
#include "../cdp/cdp_load.h"
#include "../cdp/utils.h"

#include "rf_avp.h"
#include "../charging.h"


int ShAnserHandler(AAAMessage *response, AAATransaction *t);
int ShFailureHandler(AAATransaction *t, int reason);

AAAMessage* ShRequestHandler(AAAMessage *request, void *param);

/* one possible Rf command from the CTF*/
					
/** ACR = Accouting Request*/
AAAMessage *Rf_ACR(int session_id, str user_subscriber, str ohost, str orealm, str drealm,
	unsigned int acc_type, str acc_number, str dhost, int crdtonce, int crdtpsec, int crdtpMb);
                  

#endif
