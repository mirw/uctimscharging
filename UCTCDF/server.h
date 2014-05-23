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

/*
 * $id$ server.h $date $author$ Dragos Vingarzan dvi vingarzan@gmail.com
 *
 * Copyright (C) 2005 Fhg Fokus
 *
 */

#ifndef __SERVER_H
#define __SERVER_H

#include "cdp/peer.h"
#include "cdp/diameter.h"
#include "cdp/diameter_ims.h"


AAAMessage *send_unknown_request_answer(AAAMessage *req);

int process_incoming(peer *p,AAAMessage *msg,void* ptr);

#endif
