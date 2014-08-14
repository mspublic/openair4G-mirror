/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
    included in this distribution in the file called "COPYING". If not,
    see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

*******************************************************************************/

#ifndef _SIGNALLINGRADIOBEARER_H
#    define _SIGNALLINGRADIOBEARER_H

#    include <boost/ptr_container/ptr_map.hpp>
#    include <map>
//#        include <boost/shared_ptr.hpp>

#    include "Message.h"
#    include "LogicalChannel.h"
#    include "RadioBearer.h"
#    include "SRB-ToAddMod.h"

using namespace std;

class SignallingRadioBearer: public RadioBearer  {
    public:
        //typedef boost::shared_ptr<ENodeB> ENodeBPtr;

        SignallingRadioBearer ():m_id(0u) {};
        SignallingRadioBearer (unsigned int);
        ~SignallingRadioBearer ();

        friend inline bool operator>( const SignallingRadioBearer& l, const SignallingRadioBearer r )
        {
            return l.m_id > r.m_id;
        }
        friend inline bool operator==( const SignallingRadioBearer& l, const SignallingRadioBearer r )
        {
            return l.m_id == r.m_id;
        }


    private:
        unsigned int       m_id;


        SRB_ToAddMod_t     m_srb_to_add_mod;
};
#    endif

