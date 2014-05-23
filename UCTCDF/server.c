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

#include "server.h"

#include "cdp/receiver.h"
#include "cdp/peerstatemachine.h"
#include "cdp/diameter_api.h"
#include "cdp/diameter_ims.h"
#include "cdp/diameter.h"
#include "cdf/rf.h"


AAAMessage *send_unknown_request_answer(AAAMessage *req)
{
        AAAMessage *ans=0;
        char x[4];
        AAA_AVP *avp;

        ans = AAANewMessage(req->commandCode,req->applicationId,0,req);
        if (!ans) return 0;

        set_4bytes(x,(unsigned int) DIAMETER_UNABLE_TO_COMPLY);

        avp = AAACreateAVP(AVP_Result_Code,AAA_AVP_FLAG_MANDATORY,0,x,4,AVP_DUPLICATE_DATA);
        if (!avp) {
                LOG(L_ERR,"ERR: send_unknown_request_answer() - Failed creating avp for result code\n");
                AAAFreeMessage(&ans);
                return 0;
        }
        if (AAAAddAVPToMessage(ans,avp,ans->avpList.tail)!=AAA_ERR_SUCCESS) {
                LOG(L_ERR,"ERR: send_unknown_request_answer() - Failed adding avp to message\n");
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
	
	if(msg)
	LOG(L_INFO,"Received Diameter message from %s\n",msg->orig_host->data.s);
		
	switch(msg->applicationId){
		case IMS_Rf: 
			//LOG(L_INFO,"Offline Accounting Request received from %s\n",msg->orig_host->data.s);
			ans = Rf_ACA(msg);
		break;
		default:
		
	        LOG(L_ERR,"process_incoming(): Received unserviced AppID [%d]\n",msg->applicationId);
               ans = send_unknown_request_answer(msg);               	
	}
	if (!ans) return 0;

	AAASendMessageToPeer(ans,&(p->fqdn),0,0);
	return 1;
}
