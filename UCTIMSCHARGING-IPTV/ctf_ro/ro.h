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

#ifndef AS_RO_H
#define AS_RO_H


//#include "../cdp/diameter.h"
//#include "../cdp/diameter_api.h"
//#include "../cdp/diameter_ims.h"
//#include "../cdp/cdp_load.h"
//#include "../cdp/utils.h"

#include "ro_avp.h"
#include "../charging.h"

int RoAnserHandler(AAAMessage *response, AAATransaction *t);
int RoFailureHandler(AAATransaction *t, int reason);

AAAMessage* RoRequestHandler(AAAMessage *request, void *param);
					
/** CCR = Accouting Request*/
AAAMessage *Ro_CCR(int session_id, str user_subscriber, str ohost, str orealm, str auth_app_id,
 str service_context_id, unsigned int cc_type, str cc_number, str dhost, int crdtonce, int crdtpsec, int crdtpMb);

#endif
