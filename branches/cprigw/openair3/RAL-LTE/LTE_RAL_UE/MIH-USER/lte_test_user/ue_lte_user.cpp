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

#include <odtone/base.hpp>
#include <odtone/debug.hpp>
#include <odtone/logger.hpp>
#include <odtone/mih/request.hpp>
#include <odtone/mih/response.hpp>
#include <odtone/mih/indication.hpp>
#include <odtone/mih/confirm.hpp>
#include <odtone/mih/tlv_types.hpp>
#include <odtone/sap/user.hpp>

#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <map>
#include <time.h>

///////////////////////////////////////////////////////////////////////////////

// Definition of the scenario to execute
#define NB_OF_RESOURCES		4	// Should not exceed mih_user::_max_link_action_requests
//#define SCENARIO_1	// Sequentially activate and deactivate each resource
//#define SCENARIO_2	// Activate all resources, then deactivate all resources
#define NUM_PARM_REPORT 10

///////////////////////////////////////////////////////////////////////////////
// The scenario coded in this MIH-USER demo is the following
//  +--------+                                             +-----+                                             +---------+
//  |MIH_USER|                                             |MIH-F|                                             |LINK_SAP |
//  +---+----+                                             +--+--+                                             +----+----+
//      |                                                     |                                                     |
//     ...          (start of MIH-F here)                    ...                                                   ...
//      |                                                     |---------- Link_Capability_Discover.request --------X|
//     ...          (start of LINK_SAP here)                 ...                                                   ...
//      |                                                     |<--------- Link_Register.indication -----------------|
//      |                                                     |---------- Link_Capability_Discover.request -------->|
//      |                                                     |<--------- Link_Capability_Discover.confirm ---------|
//      |                                                     |                                                     |
//     ...          (start of MIH USER here)                 ...                                                   ...
//      |---------- MIH_User_Register.indication ------------>| (supported_commands)                                |
//      |                                                     |                                                     |
//      |---------- MIH_Capability_Discover.request --------->|                                                     |
//      |<--------- MIH_Capability_Discover.confirm ----------|                                                     |
//      |                                                     |                                                     |
//      |---------- MIH_Event_Subscribe.request ------------->|---------- Link_Event_Subscribe.request ------------>|
//      |<--------- MIH_Event_Subscribe.confirm --------------|<--------- Link_Event_Subscribe.confirm -------------|
//      |                                                     |                                                     |
//      |---------- MIH_Link_Actions.request ---------------->|---------- Link_Actions.request -------------------->|
//      |            (POWER UP + SCAN)                        |             (POWER UP + SCAN)                       |
//     ...                                                   ...                                                   ...
//      |                                                     |                                                     |
//      |<--------- Link_Detected.indication -----------------|<--------- Link_Detected.indication -----------------|
//      |                                                     |                                                     |
//      |<--------- Link_Up.indication -----------------------|<--------- Link_Up.indication -----------------------|
//      |                                                     |    RRC Connection Reestablishment notification)     |
//      |                                                     |                                                     |
//      |---------- MIH_Link_Configure_Thresholds.request --->|---------- Link_Configure_Thresholds.request ------->|
//      |                                                     |                                                     |
//      |<--------- Link_Up.indication -----------------------|<--------- Link_Up.indication -----------------------|
//      |                                                     |     (RRC Connection reconfiguration notification)   |
//      |<--------- MIH_Link_Configure_Thresholds.confirm ----|<--------- Link_Configure_Thresholds.confirm --------|
//      |                                                     |                                                     |
//      |<--------- MIH_Link_Parameters_Report.indication ----|<--------- MIH_Link_Parameters_Report.indication ----|
//     ...                                                   ...                                                   ...
//      |<--------- MIH_Link_Actions.confirm -----------------|<--------- Link_Actions.confirm ---------------------|
//      |                (Success)                            |                                                     |
//      |<--------- MIH_Link_Parameters_Report.indication ----|<--------- MIH_Link_Parameters_Report.indication ----|
//     ...                                                   ...                                                   ...
//      |<--------- MIH_Link_Parameters_Report.indication ----|<--------- MIH_Link_Parameters_Report.indication ----|
//     ...                                                   ...                                                   ...
//      |<--------- MIH_Link_Parameters_Report.indication ----|<--------- MIH_Link_Parameters_Report.indication ----|
//     ...                                                   ...                                                   ...
//     etc                                                   etc                                                   etc

///////////////////////////////////////////////////////////////////////////////

static const char* const kConf_MIH_Commands = "user.commands";

///////////////////////////////////////////////////////////////////////////////

namespace po = boost::program_options;

using odtone::uint;
using odtone::ushort;
using odtone::sint8;

odtone::logger log_("[mih_usr]", std::cout);

///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
void __trim(odtone::mih::octet_string &str, const char chr)
//-----------------------------------------------------------------------------
{
    str.erase(std::remove(str.begin(), str.end(), chr), str.end());
}
//-----------------------------------------------------------------------------
template <class T> std::string StringOf(T object) {
//-----------------------------------------------------------------------------
    std::ostringstream os;
    os << object;
    return(os.str());
}
//-----------------------------------------------------------------------------
std::string getTimeStamp4Log()
//-----------------------------------------------------------------------------
{
    std::stringstream ss (std::stringstream::in | std::stringstream::out);
    struct timespec time_spec;
    unsigned int time_now_micros;
    unsigned int time_now_s;
    clock_gettime (CLOCK_REALTIME, &time_spec);
    time_now_s      = (unsigned int) time_spec.tv_sec % 3600;
    time_now_micros = (unsigned int) time_spec.tv_nsec/1000;
    ss << time_now_s << ':' << time_now_micros;
    return ss.str();
}
//-----------------------------------------------------------------------------
std::string status2string(odtone::mih::status statusP){
//-----------------------------------------------------------------------------
    switch (statusP.get()) {
        case odtone::mih::status_success:                 return "SUCCESS";break;
        case odtone::mih::status_failure:                 return "UNSPECIFIED_FAILURE";break;
        case odtone::mih::status_rejected:                return "REJECTED";break;
        case odtone::mih::status_authorization_failure:   return "AUTHORIZATION_FAILURE";break;
        case odtone::mih::status_network_error:           return "NETWORK_ERROR";break;
        default:                                          return "UNKNOWN";
    }
}
//-----------------------------------------------------------------------------
std::string link_down_reason2string(odtone::mih::link_dn_reason reasonP){
//-----------------------------------------------------------------------------
    switch (reasonP.get()) {
        case odtone::mih::link_dn_reason_explicit_disconnect:       return "DN_REASON_EXPLICIT_DISCONNECT";break;
        case odtone::mih::link_dn_reason_packet_timeout:            return "DN_REASON_PACKET_TIMEOUT";break;
        case odtone::mih::link_dn_reason_no_resource:               return "DN_REASON_NO_RESOURCE";break;
        case odtone::mih::link_dn_reason_no_broadcast:              return "DN_REASON_NO_BROADCAST";break;
        case odtone::mih::link_dn_reason_authentication_failure:    return "DN_REASON_AUTHENTICATION_FAILURE";break;
        case odtone::mih::link_dn_reason_billing_failure:           return "DN_REASON_BILLING_FAILURE";break;
        default:                                                    return "DN_REASON_UNKNOWN";
    }
}
//-----------------------------------------------------------------------------
std::string link_going_down_reason2string(odtone::mih::link_gd_reason reasonP){
//-----------------------------------------------------------------------------
    switch (reasonP.get()) {
        case odtone::mih::link_gd_reason_explicit_disconnect:       return "GD_REASON_EXPLICIT_DISCONNECT";break;
        case odtone::mih::link_gd_reason_link_parameter_degrading:  return "GD_REASON_PARAMETER_DEGRADING";break;
        case odtone::mih::link_gd_reason_low_power:                 return "GD_REASON_LOW_POWER";break;
        case odtone::mih::link_gd_reason_no_resource:               return "GD_REASON_NO_RESOURCE";break;
        default:                                                    return "GD_REASON_UNKNOWN";
    }
}
//-----------------------------------------------------------------------------
std::string evt2string(odtone::mih::mih_evt_list evtP){
//-----------------------------------------------------------------------------
    std::string s=std::string(" ");
    if(evtP.get(odtone::mih::mih_evt_link_detected))            s += "DETECTED ";
    if(evtP.get(odtone::mih::mih_evt_link_up))                  s += "UP ";
    if(evtP.get(odtone::mih::mih_evt_link_down))                s += "DOWN ";
    if(evtP.get(odtone::mih::mih_evt_link_parameters_report))   s += "PARAMETERS_REPORT ";
    if(evtP.get(odtone::mih::mih_evt_link_going_down))          s += "GOING_DOWN ";
    if(evtP.get(odtone::mih::mih_evt_link_handover_imminent))   s += "HANDOVER_IMMINENT ";
    if(evtP.get(odtone::mih::mih_evt_link_handover_complete))   s += "HANDOVER_COMPLETE ";
    if(evtP.get(odtone::mih::mih_evt_link_pdu_transmit_status)) s += "PDU_TRANSMIT_STATUS ";
    return s;
}
//-----------------------------------------------------------------------------
std::string cmd2string(odtone::mih::mih_cmd_list cmdP){
//-----------------------------------------------------------------------------
    std::string s=std::string(" ");
    if(cmdP.get(odtone::mih::mih_cmd_link_get_parameters))       s += "Link_Get_Parameters ";
    if(cmdP.get(odtone::mih::mih_cmd_link_configure_thresholds)) s += "Link_Configure_Thresholds ";
    if(cmdP.get(odtone::mih::mih_cmd_link_actions))              s += "Link_Actions ";
    if(cmdP.get(odtone::mih::mih_cmd_net_ho_candidate_query))    s += "Net_HO_Candidate_Query ";
    if(cmdP.get(odtone::mih::mih_cmd_net_ho_commit))             s += "Net_HO_Commit ";
    if(cmdP.get(odtone::mih::mih_cmd_n2n_ho_query_resources))    s += "N2N_HO_Query_Resources ";
    if(cmdP.get(odtone::mih::mih_cmd_n2n_ho_commit))             s += "N2N_HO_Commit ";
    if(cmdP.get(odtone::mih::mih_cmd_n2n_ho_complete))           s += "N2N_HO_Complete ";
    if(cmdP.get(odtone::mih::mih_cmd_mn_ho_candidate_query))     s += "MN_HO_Candidate_Query ";
    if(cmdP.get(odtone::mih::mih_cmd_mn_ho_commit))              s += "MN_HO_Commit ";
    if(cmdP.get(odtone::mih::mih_cmd_mn_ho_complete))            s += "MN_HO_Complete ";
    return s;
}
//-----------------------------------------------------------------------------
std::string link_type2string(const odtone::mih::link_type& lt)
//-----------------------------------------------------------------------------
{
    switch (lt.get()) {
	case odtone::mih::link_type_gsm:            return "GSM";            break;
	case odtone::mih::link_type_gprs:           return "GPRS";           break;
	case odtone::mih::link_type_edge:           return "EDGE";           break;
	case odtone::mih::link_type_ethernet:       return "Ethernet";       break;
	case odtone::mih::link_type_wireless_other: return "Other";          break;
	case odtone::mih::link_type_802_11:         return "IEEE 802.11";    break;
	case odtone::mih::link_type_cdma2000:       return "CDMA-2000";      break;
	case odtone::mih::link_type_umts:           return "UMTS";           break;
	case odtone::mih::link_type_cdma2000_hrpd:  return "CDMA-2000-HRPD"; break;
	case odtone::mih::link_type_lte:            return "LTE";            break;
	case odtone::mih::link_type_802_16:         return "IEEE 802.16";    break;
	case odtone::mih::link_type_802_20:         return "IEEE 802.20";    break;
	case odtone::mih::link_type_802_22:         return "IEEE 802.22";    break;
	default:                                                             break;
    }
    return "Unknown link type";
}
//-----------------------------------------------------------------------------
std::string link_addr2string(const odtone::mih::link_addr *addr)
//-----------------------------------------------------------------------------
{
    if (const odtone::mih::mac_addr                 *la = boost::get<odtone::mih::mac_addr>(addr)) {
      return la->address();
    }
    else if  (const odtone::mih::l2_3gpp_3g_cell_id *la = boost::get<odtone::mih::l2_3gpp_3g_cell_id>(addr)) {
      char plmn[16];
      sprintf(plmn, "%hhx:%hhx:%hhx", la->plmn_id[0], la->plmn_id[1], la->plmn_id[2]);
      return str(boost::format("%s %d") % plmn % la->_cell_id);
    }
    else if  (const odtone::mih::l2_3gpp_2g_cell_id *la = boost::get<odtone::mih::l2_3gpp_2g_cell_id>(addr)) {
      char plmn[16];
      sprintf(plmn, "%hhx:%hhx:%hhx", la->plmn_id[0], la->plmn_id[1], la->plmn_id[2]);
      return str(boost::format("%s %d %d") % plmn % la->_lac % la->_ci);
    }
    else if  (const odtone::mih::l2_3gpp_addr	    *la = boost::get<odtone::mih::l2_3gpp_addr>(addr)) {
      return la->value;
    }
    else if  (const odtone::mih::l2_3gpp2_addr      *la = boost::get<odtone::mih::l2_3gpp2_addr>(addr)) {
      return la->value;
    }
    else if  (const odtone::mih::other_l2_addr      *la = boost::get<odtone::mih::other_l2_addr>(addr)) {
      return la->value;
    }
    return "null";
}
//-----------------------------------------------------------------------------
std::string l2_3gpp_3g_cell_id2string(odtone::mih::l2_3gpp_3g_cell_id& addr)
//-----------------------------------------------------------------------------
{
    char buffer[256];
    int index = 0;

    index += std::sprintf(&buffer[index], "plmn: -%hhx--%hhx--%hhx-\n", addr.plmn_id[0], addr.plmn_id[1], addr.plmn_id[2]);
    index += std::sprintf(&buffer[index], "cell_id: %hhx\n", addr._cell_id);
    return buffer;
}
//-----------------------------------------------------------------------------
std::string link_id2string(odtone::mih::link_id linkP)
//-----------------------------------------------------------------------------
{
    std::string s;
    s = link_type2string(linkP.type.get()) + " " + link_addr2string(&linkP.addr);
    return s;
}
//-----------------------------------------------------------------------------
std::string ip_addr2string(odtone::mih::ip_addr ip_addrP) {
//-----------------------------------------------------------------------------
    std::string s;
    switch (ip_addrP.type()) {
	case odtone::mih::ip_addr::ipv4: s = "ipv4 "; break;
	case odtone::mih::ip_addr::ipv6: s = "ipv6 "; break;
	default: s = "Unkown type ";
    }
    s += ip_addrP.address();
    return s;
}
//-----------------------------------------------------------------------------
std::string ip_tuple2string(odtone::mih::ip_tuple ip_tupleP) {
//-----------------------------------------------------------------------------
    char buffer[128];
    std::snprintf(buffer, 128, "%s/%d", ip_addr2string(ip_tupleP.ip).c_str(), ip_tupleP.port_val);
    return buffer;
}
//-----------------------------------------------------------------------------
std::string ip_proto2string(odtone::mih::proto ip_protoP) {
//-----------------------------------------------------------------------------
    switch (ip_protoP.get()) {
	case odtone::mih::proto_tcp:	return "TCP";
	case odtone::mih::proto_udp:	return "UDP";
	default:			break;
    }
    return "Unknown IP protocol";
}
// TEMP : next 2 functions are commented to restore flow_id as a uint32
// full structure will be updated later
/*//-----------------------------------------------------------------------------
std::string flow_id2string(odtone::mih::flow_id flowP) {
//-----------------------------------------------------------------------------
    std::string s;
    odtone::mih::ip_tuple ip;
    ip = flowP.src;
    s = "SRC = " + ip_tuple2string(flowP.src);
    s += ", DST = " + ip_tuple2string(flowP.dst);
    s += ", PROTO = " + ip_proto2string(flowP.transport);
    return s;
}
//-----------------------------------------------------------------------------
std::string flow_id2string(odtone::mih::link_ac_param link_ac_paramP) {
//-----------------------------------------------------------------------------
    if (odtone::mih::resource_desc *res = boost::get<odtone::mih::resource_desc>(&link_ac_paramP.param)) {
      return flow_id2string(res->fid);
    }
    else if (odtone::mih::flow_attribute *flow = boost::get<odtone::mih::flow_attribute>(&link_ac_paramP.param)) {
      return flow_id2string(flow->id);
    }
    return "null";
}*/
//-----------------------------------------------------------------------------
std::string link_ac_result2string(odtone::mih::link_ac_result resultP)
//-----------------------------------------------------------------------------
{
    switch (resultP.get()) {
	case odtone::mih::link_ac_success:	return "SUCCESS"; break;
	case odtone::mih::link_ac_failure:	return "FAILURE"; break;
	case odtone::mih::link_ac_refused:	return "REFUSED"; break;
	case odtone::mih::link_ac_incapable:	return "INCAPABLE"; break;
	default:				break;
    }
    return "Unknown action result";
}
//-----------------------------------------------------------------------------
std::string link_actions_req2string(odtone::mih::link_action_req link_act_reqP) {
//-----------------------------------------------------------------------------
    std::string s;

    s = link_id2string(link_act_reqP.id);

    if(link_act_reqP.action.type == odtone::mih::link_ac_type_none)                      s += ", AC_TYPE_NONE";
    if(link_act_reqP.action.type == odtone::mih::link_ac_type_disconnect)                s += ", AC_TYPE_DISCONNECT";
    if(link_act_reqP.action.type == odtone::mih::link_ac_type_low_power)                 s += ", AC_TYPE_LOW_POWER";
    if(link_act_reqP.action.type == odtone::mih::link_ac_type_power_down)                s += ", AC_TYPE_POWER_DOWN";
    if(link_act_reqP.action.type == odtone::mih::link_ac_type_power_up)                  s += ", AC_TYPE_POWER_UP";
    if(link_act_reqP.action.type == odtone::mih::link_ac_type_flow_attr)                 s += ", AC_TYPE_FLOW_ATTR";
    if(link_act_reqP.action.type == odtone::mih::link_ac_type_link_activate_resources)   s += ", AC_TYPE_ACTIVATE_RESOURCES";
    if(link_act_reqP.action.type == odtone::mih::link_ac_type_link_deactivate_resources) s += ", AC_TYPE_DEACTIVATE_RESOURCES";

    if(link_act_reqP.action.attr.get(odtone::mih::link_ac_attr_data_fwd_req))            s += ", AC_ATTR_DATA_FWD_REQ";
    if(link_act_reqP.action.attr.get(odtone::mih::link_ac_attr_scan))                    s += ", AC_ATTR_SCAN";
    if(link_act_reqP.action.attr.get(odtone::mih::link_ac_attr_res_retain))              s += ", AC_ATTR_RES_RETAIN";

    s += ", " + StringOf(link_act_reqP.ex_time) + " ms";
    return s;
}
//-----------------------------------------------------------------------------
std::string net_type_addr_list2string(boost::optional<odtone::mih::net_type_addr_list> ntalP) {
//-----------------------------------------------------------------------------
    std::string                s;
    std::ostringstream         stream;
    odtone::mih::net_type_addr net_type_addr;

    for (odtone::mih::net_type_addr_list::iterator i = ntalP->begin(); i != ntalP->end(); i++)
    {
        net_type_addr = boost::get<odtone::mih::net_type_addr>(*i);
        stream << net_type_addr;
        if (i != ntalP->begin()) {
            stream <<  " / ";
        }
    }
    s =  stream.str();
    return s;
}



/**
 * Parse supported commands.
 *
 * @param cfg Configuration options.
 * @return An optional list of supported commands.
 */
boost::optional<odtone::mih::mih_cmd_list> parse_supported_commands(const odtone::mih::config &cfg)
{
	using namespace boost;

	odtone::mih::mih_cmd_list commands;

	std::map<std::string, odtone::mih::mih_cmd_list_enum> enum_map;
	enum_map["mih_link_get_parameters"]       = odtone::mih::mih_cmd_link_get_parameters;
	enum_map["mih_link_configure_thresholds"] = odtone::mih::mih_cmd_link_configure_thresholds;
	enum_map["mih_link_actions"]              = odtone::mih::mih_cmd_link_actions;
	enum_map["mih_net_ho_candidate_query"]    = odtone::mih::mih_cmd_net_ho_candidate_query;
	enum_map["mih_net_ho_commit"]             = odtone::mih::mih_cmd_net_ho_commit;
	enum_map["mih_n2n_ho_query_resources"]    = odtone::mih::mih_cmd_n2n_ho_query_resources;
	enum_map["mih_n2n_ho_commit"]             = odtone::mih::mih_cmd_n2n_ho_commit;
	enum_map["mih_n2n_ho_complete"]           = odtone::mih::mih_cmd_n2n_ho_complete;
	enum_map["mih_mn_ho_candidate_query"]     = odtone::mih::mih_cmd_mn_ho_candidate_query;
	enum_map["mih_mn_ho_commit"]              = odtone::mih::mih_cmd_mn_ho_commit;
	enum_map["mih_mn_ho_complete"]            = odtone::mih::mih_cmd_mn_ho_complete;

	std::string tmp = cfg.get<std::string>(kConf_MIH_Commands);
	__trim(tmp, ' ');

	char_separator<char> sep1(",");
	tokenizer< char_separator<char> > list_tokens(tmp, sep1);

	BOOST_FOREACH(std::string str, list_tokens) {
		if(enum_map.find(str) != enum_map.end()) {
			commands.set((odtone::mih::mih_cmd_list_enum) enum_map[str]);
		}
	}

	return commands;
}

///////////////////////////////////////////////////////////////////////////////
/**
 * This class provides an implementation of an IEEE 802.21 MIH-User.
 */
class mih_user : boost::noncopyable {
public:
    /**
     * Construct the MIH-User.
     *
     * @param cfg Configuration options.
     * @param io The io_service object that the MIH-User will use to
     * dispatch handlers for any asynchronous operations performed on the socket.
     */
    mih_user(const odtone::mih::config& cfg, boost::asio::io_service& io);

    /**
     * Destruct the MIH-User.
     */
    ~mih_user();

protected:
    /**
     * User registration handler.
     *
     * @param cfg Configuration options.
     * @param ec Error Code.
     */
    void user_reg_handler(const odtone::mih::config& cfg, const boost::system::error_code& ec);
    /**
     * Default MIH event handler.
     *
     * @param msg Received event notification.
     * @param ec Error code.
     */
    void event_handler(odtone::mih::message& msg, const boost::system::error_code& ec);
    /**
     * MIH receive message handler.
     *
     * @param msg Received message.
     * @param ec Error code.
     */
    void receive_handler(odtone::mih::message& msg, const boost::system::error_code& ec);

    void send_MIH_User_Register_indication(const odtone::mih::config& cfg);

    void send_MIH_Capability_Discover_request(void);
    void receive_MIH_Capability_Discover_confirm(odtone::mih::message& msg);

    void send_MIH_Event_Subscribe_request(odtone::mih::link_tuple_id& li, odtone::mih::mih_evt_list& evt);
    void receive_MIH_Event_Subscribe_confirm(odtone::mih::message& msg);

    void send_MIH_Event_Unsubscribe_request(void);
    void send_MIH_Event_Unsubscribe_request(odtone::mih::link_tuple_id& li, odtone::mih::mih_evt_list& evt);
    void receive_MIH_Event_Unsubscribe_confirm(odtone::mih::message& msg);

    void send_MIH_Link_Actions_request(const odtone::mih::link_id& link, odtone::mih::link_ac_type type);
    void receive_MIH_Link_Actions_confirm(odtone::mih::message& msg);
    void receive_MIH_Link_Parameters_Report(odtone::mih::message& msg, const boost::system::error_code& ec);

    void send_MIH_Link_Configure_Thresholds_request(odtone::mih::message& msg, const boost::system::error_code& ec);
    void receive_MIH_Link_Configure_Thresholds_confirm(odtone::mih::message& msg, const boost::system::error_code& ec);

private:
    odtone::sap::user          _mihf;    /**< User SAP helper.       */
    odtone::mih::id            _mihfid;  /**< MIHF destination ID.   */
    odtone::mih::id            _mihuserid;   /**< MIH_USER ID.       */

    odtone::mih::ip_addr       _mihf_ip;      /**< MIHF IP address        */
    odtone::mih::port          _mihf_lport;   /**< MIHF local port number */

    odtone::mih::link_id_list  _link_id_list;    /**< List of network link identifiers   */
    odtone::mih::mih_evt_list  _subs_evt_list;   /**< List of subscribed link events */

    odtone::mih::link_ac_type  _last_link_action_type;
    odtone::uint               _current_link_action_request, _nb_of_link_action_requests;
    odtone::uint               link_threshold_request, link_measures_request, link_measures_counter;
    odtone::mih::link_id       rcv_link_id;

    static const odtone::uint  _max_link_action_requests = 4;
    odtone::uint               _num_thresholds_request;

    void receive_MIH_Link_Detected_indication(odtone::mih::message& msg);
    void send_MIH_Link_Action_Power_Up_plus_scan_request(const odtone::mih::link_id& link);
    void receive_MIH_Link_Up_indication(odtone::mih::message& msg);

    void receive_MIH_Link_Down_indication(odtone::mih::message& msg);

    void receive_MIH_Link_Going_Down_indication(odtone::mih::message& msg);

};

//-----------------------------------------------------------------------------
mih_user::mih_user(const odtone::mih::config& cfg, boost::asio::io_service& io)
  : _mihf(cfg, io, boost::bind(&mih_user::event_handler, this, _1, _2)),
    _last_link_action_type(odtone::mih::link_ac_type_none),
    _current_link_action_request(0), _nb_of_link_action_requests(NB_OF_RESOURCES), _num_thresholds_request(0)
//-----------------------------------------------------------------------------
{
    odtone::mih::octet_string user_id = cfg.get<odtone::mih::octet_string>(odtone::sap::kConf_MIH_SAP_id);
    _mihuserid.assign(user_id.c_str());

    odtone::mih::octet_string dest_id = cfg.get<odtone::mih::octet_string>(odtone::sap::kConf_MIH_SAP_dest);
    _mihfid.assign(dest_id.c_str());

    odtone::mih::octet_string src = cfg.get<odtone::mih::octet_string>(odtone::sap::kConf_MIHF_Ip);
    boost::asio::ip::address ip = boost::asio::ip::address::from_string(src);
    if (ip.is_v4()) {
        odtone::mih::ip_addr ip_addr(odtone::mih::ip_addr::ipv4, src);
        _mihf_ip = ip_addr;
    } else if (ip.is_v6()) {
        odtone::mih::ip_addr ip_addr(odtone::mih::ip_addr::ipv6, src);
        _mihf_ip = ip_addr;
    }

    _mihf_lport = cfg.get<odtone::mih::port>(odtone::sap::kConf_MIHF_Local_Port);

    //_nb_of_link_action_requests = NB_OF_RESOURCES;
    if (_nb_of_link_action_requests > _max_link_action_requests) {
        _nb_of_link_action_requests = _max_link_action_requests;
    }

    _link_id_list.clear();
    _subs_evt_list.clear();
    link_threshold_request = 0;
    link_measures_request =0;
    link_measures_counter =0;
    log_(0, "[MSC_NEW]["+getTimeStamp4Log()+"][MIH-USER="+_mihuserid.to_string()+"]\n");

    // Send MEDIEVAL specific MIH_User_Register.indication message to the MIH-F
    mih_user::send_MIH_User_Register_indication(cfg);
}

//-----------------------------------------------------------------------------
mih_user::~mih_user()
//-----------------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------------
void mih_user::user_reg_handler(const odtone::mih::config& cfg, const boost::system::error_code& ec)
//-----------------------------------------------------------------------------
{
     log_(0, "MIH-User register result: ", ec.message(), "\n");

    //
    // Let's fire a capability discover request to get things moving
    //
    mih_user::send_MIH_Capability_Discover_request();
}

//-----------------------------------------------------------------------------
void mih_user::event_handler(odtone::mih::message& msg, const boost::system::error_code& ec)
//-----------------------------------------------------------------------------
{
    if (ec) {
        log_(0, __FUNCTION__, " error: ", ec.message());
        return;
    }

    switch (msg.mid()) {
    case odtone::mih::indication::link_detected:
        mih_user::receive_MIH_Link_Detected_indication(msg);
        break;

    case odtone::mih::indication::link_up:
        mih_user::receive_MIH_Link_Up_indication(msg);
        if (_num_thresholds_request == 0) {
            mih_user::send_MIH_Link_Configure_Thresholds_request(msg, ec);
            _num_thresholds_request += 1;;
        }
        break;

    case odtone::mih::indication::link_down:
        mih_user::receive_MIH_Link_Down_indication(msg);
        break;

    case odtone::mih::indication::link_going_down:
        mih_user::receive_MIH_Link_Going_Down_indication(msg);
        break;

    case odtone::mih::indication::link_handover_imminent:
        log_(0, "MIH-User has received a local event \"link_handover_imminent\"");
        break;

    case odtone::mih::indication::link_handover_complete:
        log_(0, "MIH-User has received a local event \"link_handover_complete\"");
        break;

    case odtone::mih::indication::link_parameters_report:
        //log_(0, "MIH-User has received a local event \"link_parameters_report\"");
        mih_user::receive_MIH_Link_Parameters_Report(msg, ec);
        /*if (link_threshold_request == 0){
            mih_user::send_MIH_Link_Configure_Thresholds_request(msg, ec);
            link_threshold_request =1;
        } else if (link_threshold_request == 1){
            link_measures_counter ++;
            // Stop measures after 5 reports
            if (link_measures_counter == NUM_PARM_REPORT){
                mih_user::send_MIH_Link_Configure_Thresholds_request(msg, ec);
            }
        }*/
        break;

    case odtone::mih::indication::link_pdu_transmit_status:
        log_(0, "MIH-User has received a local event \"link_pdu_transmit_status\"");
        break;

    case odtone::mih::confirm::link_configure_thresholds:
        mih_user::receive_MIH_Link_Configure_Thresholds_confirm(msg, ec);
        break;

    default:
        log_(0, "MIH-User has received UNKNOWN local event");
        break;
    }
}

//-----------------------------------------------------------------------------
void mih_user::receive_handler(odtone::mih::message& msg, const boost::system::error_code& ec)
//-----------------------------------------------------------------------------
{
    if (ec) {
        log_(0, __FUNCTION__, " error: ", ec.message());
        return;
    }

    switch (msg.mid()) {

    case odtone::mih::confirm::capability_discover:
        mih_user::receive_MIH_Capability_Discover_confirm(msg);
        break;

    case odtone::mih::confirm::event_subscribe:
        mih_user::receive_MIH_Event_Subscribe_confirm(msg);
        break;

    case odtone::mih::confirm::event_unsubscribe:
        mih_user::receive_MIH_Event_Unsubscribe_confirm(msg);
        break;

    case odtone::mih::confirm::link_actions:
        mih_user::receive_MIH_Link_Actions_confirm(msg);
        break;

    default:
        log_(0, "MIH-User has received UNKNOWN message (", msg.mid(), ")\n");
        break;
	}
}

//-----------------------------------------------------------------------------
void mih_user::receive_MIH_Link_Detected_indication(odtone::mih::message& msg)
//-----------------------------------------------------------------------------
{
    log_(0, "MIH_Link_Detected.indication - RECEIVED - Begin\n");
    odtone::mih::link_det_info                ldi;
    odtone::mih::link_det_info_list           ldil;
    odtone::mih::link_det_info_list::iterator it_ldil;
    odtone::mih::link_id                      lid;

    msg >> 	odtone::mih::indication() & odtone::mih::tlv_link_det_info_list(ldil);

    log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ msg.source().to_string() +"][--- MIH_Link_Detected.indication --->]["+msg.destination().to_string()+"]\n");
    for(it_ldil = ldil.begin(); it_ldil != ldil.end(); it_ldil++) {
            ldi = *it_ldil;
            log_(0, "\tMIH_Link_Detected.indication - network_id:........", ldi.network_id.c_str());
            log_(0, "\tMIH_Link_Detected.indication - net_aux_id:........", ldi.net_aux_id.c_str());
            log_(0, "\tMIH_Link_Detected.indication - sig_strength:......TO DO");//, ldi.signal);
            log_(0, "\tMIH_Link_Detected.indication - sinr:..............", ldi.sinr);
            log_(0, "\tMIH_Link_Detected.indication - data_rate:.........", ldi.data_rate);
            log_(0, "\tMIH_Link_Detected.indication - data_rate:.........", ldi.data_rate);
            log_(0, "\tMIH_Link_Detected.indication - mih_capabilities:..", ldi.data_rate);
            log_(0, "\tMIH_Link_Detected.indication - net_capabilities:..TO DO");//, ldi.net_capabilities);

    }
    // Display message parameters
    // TODO: for each link_det_info in the list {display LINK_DET_INFO}

    // send Link_Action / Power Up
    lid.type = odtone::mih::link_type_lte;
    lid.addr = ldi.id.addr;
    //send_MIH_Link_Action_Power_Up_plus_scan_request(lid);
    log_(0, "MIH_Link_Detected.indication - End\n");
}


//-----------------------------------------------------------------------------
void mih_user::send_MIH_Link_Action_Power_Up_plus_scan_request(const odtone::mih::link_id& link)
//-----------------------------------------------------------------------------
{
    odtone::mih::message          m;
    odtone::mih::link_action_list lal;
    odtone::mih::link_action_req  link_act_req;
    //struct null                   n;

    link_act_req.id               = link;
    link_act_req.action.type      = odtone::mih::link_ac_type_power_up;
    link_act_req.action.attr.clear();
    link_act_req.action.attr.set(odtone::mih::link_ac_attr_scan);

    link_act_req.ex_time            = 5000; // in ms

    lal.push_back(link_act_req);

    m << odtone::mih::request(odtone::mih::request::link_actions)
      & odtone::mih::tlv_link_action_list(lal);
    m.source(_mihuserid);
    m.destination(_mihfid);

    log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ m.source().to_string() +"][--- MIH_Link_Actions.request\\n"+link_actions_req2string(link_act_req)+" --->]["+m.destination().to_string()+"]\n");

    _mihf.async_send(m, boost::bind(&mih_user::receive_handler, this, _1, _2));

    log_(0, "MIH-User has sent Link_Actions.request to ", m.destination().to_string());
    log_(0, "   - LINK_ID - Link identifier:  ", link_id2string(link).c_str());
    log_(0, "   - LINK_ACTIONS - Link Actions: " + link_actions_req2string(link_act_req) + "\n");

    log_(0, "MIH_Link_Action_Power_Up_request - SENT\n");
}

//-----------------------------------------------------------------------------
void mih_user::receive_MIH_Link_Up_indication(odtone::mih::message& msg)
//-----------------------------------------------------------------------------
{
    log_(0, "MIH_Link_Up.indication - RECEIVED - Begin\n");

    odtone::mih::link_tuple_id link;
//        odtone::mih::tlv_old_access_router oldAR;

    msg >> 	odtone::mih::indication() & odtone::mih::tlv_link_identifier(link);
//          & odtone::mih::tlv_old_access_router(oar);

    log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ msg.source().to_string() +"][--- MIH_Link_Up.indication --->]["+msg.destination().to_string()+"]\n");

    // Display message parameters
    log_(0, "   - LINK_ID - Link identifier:  ", link_id2string(link).c_str(), "\n");

    log_(0, "MIH_Link_Up.indication - End\n");
}

//-----------------------------------------------------------------------------
void mih_user::receive_MIH_Link_Down_indication(odtone::mih::message& msg)
//-----------------------------------------------------------------------------
{
    odtone::mih::link_tuple_id               link;
    boost::optional<odtone::mih::link_addr>  addr;
    odtone::mih::link_dn_reason              ldr;

    log_(0, "MIH_Link_Down.indication - RECEIVED - Begin\n");
    msg >> odtone::mih::indication()
      & odtone::mih::tlv_link_identifier(link)
      & odtone::mih::tlv_old_access_router(addr)
      & odtone::mih::tlv_link_dn_reason(ldr);

//  log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ msg.source().to_string() +"][--- MIH_Link_Down.indication\\n"+link_down_reason2string(ldr).c_str()+" --->]["+msg.destination().to_string()+"]\n");

    //Display message parameters
    log_(0, "   - LINK_ID - Link identifier:  ", link_id2string(link).c_str(), "\n");
//  log_(0, "   - LINK_DN_REASON - Link down reason:  ", link_down_reason2string(ldr).c_str(), "\n");

    log_(0, "MIH_Link_Down.indication - End\n");
}

//-----------------------------------------------------------------------------
void mih_user::receive_MIH_Link_Going_Down_indication(odtone::mih::message& msg)
//-----------------------------------------------------------------------------
{
    log_(0, "MIH_Link_Going_Down.indication - RECEIVED - Begin\n");

    odtone::mih::link_tuple_id   link;
    odtone::mih::link_gd_reason  lgd;
    odtone::mih::link_ac_ex_time ex_time;

    msg >> odtone::mih::indication()
      & odtone::mih::tlv_link_identifier(link)
      & odtone::mih::tlv_time_interval(ex_time)
      & odtone::mih::tlv_link_gd_reason(lgd);

    log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ msg.source().to_string() +"][--- MIH_Link_Going_Down.indication\\n"+link_going_down_reason2string(lgd).c_str()+" --->]["+msg.destination().to_string()+"]\n");

    // Display message parameters
    log_(0, "   - LINK_ID - Link identifier:  ", link_id2string(link).c_str());
    log_(0, "   - Time Interval:", (ex_time/256));
    log_(0, "   - LINK_GD_REASON - Link going down reason: ", link_going_down_reason2string(lgd).c_str(), "\n");

    log_(0, "MIH_Link_Going_Down.indication - End\n");
}

//-----------------------------------------------------------------------------
void mih_user::receive_MIH_Link_Parameters_Report(odtone::mih::message& msg, const boost::system::error_code& ec)
//-----------------------------------------------------------------------------
{
    odtone::mih::link_tuple_id link;
    odtone::mih::link_param_rpt_list lprl;

    msg >> odtone::mih::indication()
           & odtone::mih::tlv_link_identifier(link)
           & odtone::mih::tlv_link_param_rpt_list(lprl);

    log_(0, "");
    log_(0, "MIH_Link_Parameters_Report.indication - RECEIVED - Begin");
    log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ msg.source().to_string() +"][--- MIH_Link_Parameters_Report.indication --->]["+msg.destination().to_string()+"]\n");
    log_(0, "  - LINK_TUPLE_ID - Link identifier:  ", link_id2string(link).c_str());
    log_(0, "MIH_Link_Parameters_Report.indication - End");
}


//-----------------------------------------------------------------------------
void mih_user::send_MIH_User_Register_indication(const odtone::mih::config& cfg)
//-----------------------------------------------------------------------------
{
	odtone::mih::message m;
	boost::optional<odtone::mih::mih_cmd_list> supp_cmd = parse_supported_commands(cfg);

	m << odtone::mih::indication(odtone::mih::indication::user_register)
	    & odtone::mih::tlv_command_list(supp_cmd);
	m.source(_mihuserid);
	m.destination(_mihfid);

	log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ m.source().to_string() +"][--- MIH_User_Register.indication --->]["+m.destination().to_string()+"]\n");

	_mihf.async_send(m, boost::bind(&mih_user::user_reg_handler, this, boost::cref(cfg), _2));

	log_(0, "MIH_User_Register.indication - SENT (towards its local MIHF)\n");
}

//-----------------------------------------------------------------------------
void mih_user::send_MIH_Capability_Discover_request(void)
//-----------------------------------------------------------------------------
{
	odtone::mih::message m;
	m << odtone::mih::request(odtone::mih::request::capability_discover);
	m.source(_mihuserid);
	m.destination(_mihfid);

	log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ m.source().to_string() +"][--- MIH_Capability_Discover.request --->]["+m.destination().to_string()+"]\n");

	_mihf.async_send(m, boost::bind(&mih_user::receive_handler, this, _1, _2));

	log_(0, "MIH_Capability_Discover.request - SENT (towards its local MIHF)\n");
}

//-----------------------------------------------------------------------------
void mih_user::receive_MIH_Capability_Discover_confirm(odtone::mih::message& msg)
//-----------------------------------------------------------------------------
{
    log_(0, "MIH_Capability_Discover.confirm - RECEIVED - Begin\n");

    odtone::mih::status                              st;
    boost::optional<odtone::mih::net_type_addr_list> ntal;
    boost::optional<odtone::mih::mih_evt_list>       evt;
    boost::optional<odtone::mih::mih_cmd_list>       cmd;

    msg >> odtone::mih::confirm()
            & odtone::mih::tlv_status(st)
            & odtone::mih::tlv_net_type_addr_list(ntal)
            & odtone::mih::tlv_event_list(evt)
            & odtone::mih::tlv_command_list(cmd);

    log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ msg.source().to_string() +"][--- MIH_Capability_Discover.confirm"+
         "\\nstatus="+status2string(st).c_str()+
         "\\nEvent list="+evt2string(evt.get()).c_str()+
         "\\nNet type addr list=" + net_type_addr_list2string(ntal).c_str()+
         " --->]["+msg.destination().to_string()+"]\n");

    // Display message parameters
    log_(0, "  - STATUS: ", status2string(st).c_str(), " (", st.get(), ")");
    if (evt) {
      log_(0, "  - MIH_EVT_LIST - Event List:  ", evt2string(evt.get()).c_str());
    }
    if (cmd) {
      log_(0, "  - MIH_CMD_LIST - Command List:  ", cmd2string(cmd.get()).c_str());
    }
    if (ntal) {
      log_(0, "  - LIST(NET_TYPE_ADDR) - Network Types and Link Address: ", net_type_addr_list2string(ntal).c_str());
          //Store link address
          for (odtone::mih::net_type_addr_list::iterator i = ntal->begin(); i != ntal->end(); i++)
          {
            rcv_link_id.addr = i->addr;
            rcv_link_id.type = boost::get<odtone::mih::link_type>(i->nettype.link);
          }
    }
    log_(0, "");

    //
    // event subscription
    //
    // For every interface the MIHF sent in the
    // Capability_Discover.response send an Event_Subscribe.request
    // for all availabe events
    //
    if (ntal && evt) {
        _subs_evt_list = evt.get(); // save the list of subscribed link events
        for (odtone::mih::net_type_addr_list::iterator i = ntal->begin(); i != ntal->end(); i++) {
            if (i->nettype.link.which() == 1)
            {
                odtone::mih::link_tuple_id li;

                li.addr = i->addr;
                li.type = boost::get<odtone::mih::link_type>(i->nettype.link);
                _link_id_list.push_back(li);    // save the link identifier of the network interface

                mih_user::send_MIH_Event_Subscribe_request(li, evt.get());
            }
        }
    }

    log_(0, "MIH_Capability_Discover.confirm - End\n");
}

//-----------------------------------------------------------------------------
void mih_user::send_MIH_Event_Subscribe_request(odtone::mih::link_tuple_id& li, odtone::mih::mih_evt_list& evt)
//-----------------------------------------------------------------------------
{
    odtone::mih::message m;

    m << odtone::mih::request(odtone::mih::request::event_subscribe)
      & odtone::mih::tlv_link_identifier(li)
      & odtone::mih::tlv_event_list(evt);
    m.source(_mihuserid);
    m.destination(_mihfid);

    log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ m.source().to_string() +"][--- MIH_Event_Subscribe.request"+
         "\\nLink="+link_id2string(li).c_str()+
         "\\nEvent list="+evt2string(evt).c_str()+
         " --->]["+m.destination().to_string()+"]\n");

    _mihf.async_send(m, boost::bind(&mih_user::receive_handler, this, _1, _2));

    log_(0, "MIH-User has sent Event_Subscribe.request to ", m.destination().to_string());
    log_(0, "  - LINK_TUPLE_ID - Link identifier:  ", link_id2string(li).c_str());
    log_(0, "  - MIH_EVT_LIST - Event List:  ", evt2string(evt).c_str(), "\n");

    log_(0, "MIH_Event_Subscribe.request - SENT\n");
}

//-----------------------------------------------------------------------------
void mih_user::receive_MIH_Event_Subscribe_confirm(odtone::mih::message& msg)
//-----------------------------------------------------------------------------
{
    log_(0, "MIH_Event_Subscribe.confirm(", msg.tid(), ") - RECEIVED - Begin\n");

    odtone::mih::status                        st;
    odtone::mih::link_tuple_id                 link;
    boost::optional<odtone::mih::mih_evt_list> evt;

    msg >> odtone::mih::confirm()
      & odtone::mih::tlv_status(st)
      & odtone::mih::tlv_link_identifier(link)
      & odtone::mih::tlv_event_list(evt);

    if (evt) {
        log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ msg.source().to_string() +"][--- MIH_Event_Subscribe.confirm"+
             "\\nstatus="+status2string(st).c_str()+
             "\\nLink="+link_id2string(link).c_str()+
             "\\nEvent list="+evt2string(evt.get()).c_str()+
             " --->]["+msg.destination().to_string()+"]\n");
    } else {
        log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ msg.source().to_string() +"][--- MIH_Event_Subscribe.confirm"+
             "\\nstatus="+status2string(st).c_str()+
             "\\nLink="+link_id2string(link).c_str()+
             " --->]["+msg.destination().to_string()+"]\n");
    }

    // Display message parameters
    log_(0, "  - STATUS: ", status2string(st).c_str(), " (", st.get(), ")");
    log_(0, "  - LINK_TUPLE_ID - Link identifier:  ", link_id2string(link).c_str());
    if (evt) {
      log_(0, "  - MIH_EVT_LIST - Event List:  ", evt2string(evt.get()).c_str());
    }
    log_(0, "");

    //mih_user::send_MIH_Link_Actions_request(link, odtone::mih::link_ac_type_link_activate_resources);
    //log_(0, "TEMP : Resource scenario deactivated\n");

    log_(0, "MIH_Event_Subscribe.confirm - End\n");
    mih_user::send_MIH_Link_Action_Power_Up_plus_scan_request(link);

}

//-----------------------------------------------------------------------------
void mih_user::send_MIH_Event_Unsubscribe_request(void)
//-----------------------------------------------------------------------------
{
    odtone::mih::link_tuple_id li;

    // For every interface the MIH user received in the
    // Capability_Discover.confirm, send an Event_Unsubscribe.request
    // for all subscribed events
    for (odtone::mih::link_id_list::iterator i = _link_id_list.begin(); i != _link_id_list.end(); i++) {
        li.type = i->type;
        li.addr = i->addr;
        mih_user::send_MIH_Event_Unsubscribe_request(li, _subs_evt_list);
    }
}

//-----------------------------------------------------------------------------
void mih_user::send_MIH_Event_Unsubscribe_request(odtone::mih::link_tuple_id& li, odtone::mih::mih_evt_list& evt)
//-----------------------------------------------------------------------------
{
    odtone::mih::message m;

    m << odtone::mih::request(odtone::mih::request::event_unsubscribe)
      & odtone::mih::tlv_link_identifier(li)
      & odtone::mih::tlv_event_list(evt);
    m.source(_mihuserid);
    m.destination(_mihfid);

    log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ m.source().to_string() +"][--- MIH_Event_Unsubscribe.request"+
         "\\nLink="+link_id2string(li).c_str()+
         "\\nEvent list="+evt2string(evt).c_str()+
         " --->]["+m.destination().to_string()+"]\n");

    _mihf.async_send(m, boost::bind(&mih_user::receive_handler, this, _1, _2));

    log_(0, "MIH-User has sent Event_Unsubscribe.request to ", m.destination().to_string());
    log_(0, "  - LINK_TUPLE_ID - Link identifier:  ", link_id2string(li).c_str());
    log_(0, "  - MIH_EVT_LIST - Event List:  ", evt2string(evt).c_str(), "\n");

    log_(0, "MIH_Event_Unsubscribe.request - SENT\n");
}

//-----------------------------------------------------------------------------
void mih_user::receive_MIH_Event_Unsubscribe_confirm(odtone::mih::message& msg)
//-----------------------------------------------------------------------------
{
    log_(0, "MIH_Event_Unsubscribe.confirm(", msg.tid(), ") - RECEIVED - Begin\n");

    odtone::mih::status st;
    odtone::mih::link_tuple_id link;
    boost::optional<odtone::mih::mih_evt_list> evt;

    msg >>  odtone::mih::confirm()
      & odtone::mih::tlv_status(st)
      & odtone::mih::tlv_link_identifier(link)
      & odtone::mih::tlv_event_list(evt);

    if (evt) {
        log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ msg.source().to_string() +"][--- MIH_Event_Unsubscribe.confirm"+
             "\\nstatus="+status2string(st).c_str()+
             "\\nLink="+link_id2string(link).c_str()+
             "\\nEvent list="+evt2string(evt.get()).c_str()+
             " --->]["+msg.destination().to_string()+"]\n");
    } else {
        log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ msg.source().to_string() +"][--- MIH_Event_Unsubscribe.confirm"+
             "\\nstatus="+status2string(st).c_str()+
             "\\nLink="+link_id2string(link).c_str()+
             " --->]["+msg.destination().to_string()+"]\n");
    }

    // Display message parameters
    log_(0, "  - STATUS: ", status2string(st).c_str(), " (", st.get(), ")");
    log_(0, "  - LINK_TUPLE_ID - Link identifier:  ", link_id2string(link).c_str());
    if (evt) {
      log_(0, "  - MIH_EVT_LIST - Event List:  ", evt2string(evt.get()).c_str());
    }
    log_(0, "");

    log_(0, "MIH_Event_Unsubscribe.confirm - End");
}

//-----------------------------------------------------------------------------
void mih_user::send_MIH_Link_Actions_request(const odtone::mih::link_id& link, odtone::mih::link_ac_type type)
//-----------------------------------------------------------------------------
{
    odtone::mih::message m;
    odtone::mih::link_action_list lal;
    odtone::mih::link_action_req link_act_req;

    link_act_req.id = link;
    link_act_req.action.type = type;

    _last_link_action_type = type;

    // Initialize resource parameters
    odtone::mih::resource_desc res;

    res.lid       = link;       // Link identifier
    res.data_rate = 128000;     // bit rate
    res.jumbo     = false;      // jumbo disable
    res.multicast = false;      // multicast disable

    odtone::mih::qos qos;       // Class Of Service
    qos.value     = 56;
    res.qos_val   = qos;
    res.fid = 555 + _current_link_action_request;

// 	// Flow identifier
// 	res.fid.src.ip = _mihf_ip;
// 	res.fid.src.port_val = _mihf_lport;
// 
// 	if (mih_user::_current_link_action_request == 0) {
// 	  res.fid.dst.ip = odtone::mih::ip_addr(odtone::mih::ip_addr::ipv6,
// 						"2001:0660:0382:0014:0335:0600:8014:9150"); // DUMMY
// 	}
// 	else if (mih_user::_current_link_action_request == 1) {
// 	  res.fid.dst.ip = odtone::mih::ip_addr(odtone::mih::ip_addr::ipv6,
// 						"2001:0660:0382:0014:0335:0600:8014:9151"); // DUMMY
// 	}
// 	else if (mih_user::_current_link_action_request == 2) {
// 	  res.fid.dst.ip = odtone::mih::ip_addr(odtone::mih::ip_addr::ipv6,
// 						"FF3E:0020:2001:0DB8:0000:0000:0000:0043"); // DUMMY
// 	  res.multicast = true;
// 	}
// 	else if (mih_user::_current_link_action_request == 3) {
// 	  res.fid.dst.ip = odtone::mih::ip_addr(odtone::mih::ip_addr::ipv6,
// 						"2001:0660:0382:0014:0335:0600:8014:9153"); // DUMMY
// 	}
// 	res.fid.dst.port_val = 1235; // DUMMY
// 	res.fid.transport = odtone::mih::proto_udp;

    link_act_req.action.param.param = res;

    link_act_req.ex_time = 0;

    lal.push_back(link_act_req);

    m << odtone::mih::request(odtone::mih::request::link_actions)
      & odtone::mih::tlv_link_action_list(lal);
    m.source(_mihuserid);
    m.destination(_mihfid);

    log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ m.source().to_string() +"][--- MIH_Link_Actions.request\\n"+link_actions_req2string(link_act_req)+" --->]["+m.destination().to_string()+"]\n");

    _mihf.async_send(m, boost::bind(&mih_user::receive_handler, this, _1, _2));
 
    log_(0, "MIH-User has sent Link_Actions.request to ", m.destination().to_string());
    log_(0, "   - LINK_ID - Link identifier:  ", link_id2string(link).c_str());
    log_(0, "   - FLOW_ID - Flow identifier:  ", res.fid);
//TEMP  log_(0, "   - FLOW_ID - Flow identifier:  ", flow_id2string(link_act_req.action.param).c_str());
    log_(0, "   - LINK_ACTIONS - Link Actions: " + link_actions_req2string(link_act_req) + "\n");

    log_(0, "MIH_Link_Actions.request - SENT\n");
}

//-----------------------------------------------------------------------------
void mih_user::receive_MIH_Link_Actions_confirm(odtone::mih::message& msg)
//-----------------------------------------------------------------------------
{
    log_(0, "MIH_Link_Actions.confirm - RECEIVED - Begin\n");

    odtone::mih::status st;
    boost::optional<odtone::mih::link_action_rsp_list> larl;

    msg >> odtone::mih::confirm()
      & odtone::mih::tlv_status(st)
      & odtone::mih::tlv_link_action_rsp_list(larl);

    log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ msg.source().to_string() +"][--- MIH_Link_Actions.confirm"+
         "\\nstatus="+status2string(st).c_str()+
         " --->]["+msg.destination().to_string()+"]\n");

    // Display message parameters
    log_(0, "  - STATUS: ", status2string(st).c_str(), " (", st.get(), ")");
    if (larl) {
        log_(0, "  - LINK ACTION RSP LIST - Length:", larl.get().size());
        for (odtone::mih::link_action_rsp_list::iterator i = larl->begin(); i != larl->end(); i++)
        {
        log_(0, "\tLINK_ID: ", link_id2string(i->id).c_str(),
             ", LINK_AC_RESULT: ", link_ac_result2string(i->result).c_str());
        }
    }
    log_(0, "");

    // 1st scenario: Sequentially activate and deactivate each resource
#ifdef SCENARIO_1
	if (larl) {
	    odtone::mih::link_action_rsp *rsp = &larl->front();
	    if (_current_link_action_request < _nb_of_link_action_requests) {
	        if (_last_link_action_type == odtone::mih::link_ac_type_link_activate_resources) {
		    if (rsp->result.get() == odtone::mih::link_ac_success) {
		        mih_user::send_MIH_Link_Actions_request(rsp->id, odtone::mih::link_ac_type_link_deactivate_resources);
			_current_link_action_request += 1;
		    }
		}
		else if (_last_link_action_type == odtone::mih::link_ac_type_link_deactivate_resources) {
		    mih_user::send_MIH_Link_Actions_request(rsp->id, odtone::mih::link_ac_type_link_activate_resources);
		}
	    }
	    else { // Ends the scenario
	        mih_user::send_MIH_Event_Unsubscribe_request();
	    }
	}
#endif // SCENARIO_1

#ifdef SCENARIO_2
	// 2nd scenario: Activate all resources, then deactivate all resources
	if (larl.get().size() > 0) {
	    odtone::mih::link_action_rsp *rsp = &larl->front();
	    if (++_current_link_action_request < _nb_of_link_action_requests) {
		if (_last_link_action_type == odtone::mih::link_ac_type_link_activate_resources) {
		    mih_user::send_MIH_Link_Actions_request(rsp->id, odtone::mih::link_ac_type_link_activate_resources);
		}
		else if (_last_link_action_type == odtone::mih::link_ac_type_link_deactivate_resources) {
		    mih_user::send_MIH_Link_Actions_request(rsp->id, odtone::mih::link_ac_type_link_deactivate_resources);
		}
	    }
	    else if (_last_link_action_type == odtone::mih::link_ac_type_link_activate_resources) {
		_current_link_action_request = 0;
		mih_user::send_MIH_Link_Actions_request(rsp->id, odtone::mih::link_ac_type_link_deactivate_resources);
	    }
	    else { // Ends the scenario
	      mih_user::send_MIH_Event_Unsubscribe_request();
	    }
	}
#endif // SCENARIO_2

    log_(0, "MIH_Link_Actions.confirm - End\n");
}

//-----------------------------------------------------------------------------
void mih_user::send_MIH_Link_Configure_Thresholds_request(odtone::mih::message& msg, const boost::system::error_code& ec)
//-----------------------------------------------------------------------------
{
    odtone::mih::message                 m;
    odtone::mih::threshold               th;
    std::vector<odtone::mih::threshold>  thl;
    odtone::mih::link_tuple_id           lti;
    odtone::mih::l2_3gpp_addr            local_l2_3gpp_addr;
    //List of the link threshold parameters
    odtone::mih::link_cfg_param_list     lcpl;
    odtone::mih::link_cfg_param          lcp;
    odtone::mih::link_param_lte          lp;
    //odtone::mih::link_param_gen          lp;

    odtone::mih::link_param_type typr;

    log_(0,"");
    log_(0, "send_MIH_Link_Configure_Thresholds_request - Begin");

    //link_tuple_id
    lti.type = rcv_link_id.type;
    lti.addr = rcv_link_id.addr;

    //local_l2_3gpp_addr = boost::get<odtone::mih::l2_3gpp_addr>(lti.addr);


    //link_param_gen_data_rate = 0,           /**< Data rate.         */
    //link_param_gen_signal_strength = 1,     /**< Signal strength.   */
    //link_param_gen_sinr = 2,                /**< SINR.              */
    //link_param_gen_throughput = 3,          /**< Throughput.        */
    //link_param_gen_packet_error_rate = 4,   /**< Packet error rate. */
    //lp = odtone::mih::link_param_lte_bandwidth;
    lp = odtone::mih::link_param_lte_rsrp;
    lcp.type = lp;

    link_measures_request = 0;
    if ( link_measures_request ==0){
        // Set Timer Interval (in ms)
        lcp.timer_interval = 3000;
        //th_action_normal = 0,   /**< Set normal threshold.      */
        //th_action_one_shot = 1, /**< Set one-shot threshold.    */
        //th_action_cancel = 2    /**< Cancel threshold.          */
        lcp.action = odtone::mih::th_action_normal;
        link_measures_request = 1;
    } else if ( link_measures_request==1){
        // Set Timer Interval (in ms)
        lcp.timer_interval = 0;
        lcp.action = odtone::mih::th_action_cancel;
        link_measures_request = 0;
    }

    //above_threshold = 0,    /**< Above threshold.   */
    //below_threshold = 1,    /**< Below threshold.   */
    th.threshold_val = -105;
    th.threshold_x_dir = odtone::mih::threshold::above_threshold;

    thl.push_back(th);
    lcp.threshold_list = thl;
    lcpl.push_back(lcp);

    m <<  odtone::mih::request(odtone::mih::request::link_configure_thresholds)
          & odtone::mih::tlv_link_identifier(lti)
          & odtone::mih::tlv_link_cfg_param_list(lcpl);

    m.destination(msg.source());

    log_(0, "[MSC_MSG]["+getTimeStamp4Log()+"]["+ _mihuserid.to_string() +"][--- MIH_Link_Configure_Thresholds.request\\nlink="+
                // link_tupple_id2string(lti).c_str() +
                link_id2string(lti).c_str()+
                 " --->]["+_mihfid.to_string()+"]\n");

    _mihf.async_send(m, boost::bind(&mih_user::event_handler, this, _1, _2));

    log_(0, "  - LINK_TUPLE_ID - Link identifier:  ", link_id2string(lti).c_str());

    log_(0, "\t- LINK CFG PARAM LIST - Length: ", lcpl.size());

    //if(lp == odtone::mih::link_param_gen_data_rate)       {log_(0, "\t  Generic link parameter DATA RATE ");}
    //if(lp == odtone::mih::link_param_gen_signal_strength) {log_(0, "\t  Generic link parameter SIGNAL STRENGTH");}
    //if(lp == odtone::mih::link_param_gen_sinr)            {log_(0, "\t   Generic link parameter SINR");}
    //if(lp == odtone::mih::link_param_gen_throughput)      {log_(0, "\t  Generic link parameter THROUGHPUT");}
    //if(lp == odtone::mih::link_param_lte_bandwidth)       {log_(0, "\t  LTE link parameter BANDWIDTH");}
    if(lp == odtone::mih::link_param_lte_rsrp)       {log_(0, "\t  LTE link parameter LTE RSRP");}

    log_(0, "\t- TIMER INTERVAL - Value: ", lcp.timer_interval);

    if(lcp.action == odtone::mih::th_action_normal)   {log_(0, "\t  Normal Threshold");}
    if(lcp.action == odtone::mih::th_action_one_shot) {log_(0, "\t  One Shot Threshold");}
    if(lcp.action == odtone::mih::th_action_cancel)   {log_(0, "\t  Threshold to be canceled");}

    log_(0, "\t  Threshold value: ", th.threshold_val);

    if(th.threshold_x_dir == odtone::mih::threshold::below_threshold) {log_(0, "\t  Threshold direction BELOW");}
    if(th.threshold_x_dir == odtone::mih::threshold::above_threshold) {log_(0, "\t  Threshold direction ABOVE");}

    log_(0, "send_MIH_Link_Configure_Thresholds_request - End");
}

//-----------------------------------------------------------------------------
void mih_user::receive_MIH_Link_Configure_Thresholds_confirm(odtone::mih::message& msg, const boost::system::error_code& ec)
//-----------------------------------------------------------------------------
{
    log_(0, "");
    log_(0, "receive_MIH_Link_Configure_Thresholds_confirm - Begin");

    // T odtone::uint iter;
    // T odtone::mih::status st;

    //boost::optional<odtone::mih::link_cfg_status_list> lcsl;
    // Todtone::mih::link_cfg_status_list lcsl;
    // Todtone::mih::link_cfg_status lcp;
    //odtone::mih::link_param_gen lp;

    // T odtone::mih::link_tuple_id lti;

    //msg >> odtone::mih::confirm()
    //    & odtone::mih::tlv_status(st)
    //    & odtone::mih::tlv_link_identifier(lti)
    //    & odtone::mih::tlv_link_cfg_status_list(lcsl);


    log_(0, "receive_MIH_Link_Configure_Thresholds_confirm - End");
    log_(0,"");
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
//-----------------------------------------------------------------------------
{
    odtone::setup_crash_handler();

    try {
        boost::asio::io_service ios;

        // declare MIH Usr available options
        po::options_description desc(odtone::mih::octet_string("MIH Usr Configuration"));
        desc.add_options()
            ("help", "Display configuration options")
            (odtone::sap::kConf_File, po::value<std::string>()->default_value("ue_lte_user.conf"), "Configuration file")
            (odtone::sap::kConf_Receive_Buffer_Len, po::value<uint>()->default_value(4096),        "Receive buffer length")
            (odtone::sap::kConf_Port, po::value<ushort>()->default_value(1235),                    "Listening port")
            (odtone::sap::kConf_MIH_SAP_id, po::value<std::string>()->default_value("user"),       "MIH-User ID")
            (kConf_MIH_Commands, po::value<std::string>()->default_value(""),                      "MIH-User supported commands")
            (odtone::sap::kConf_MIHF_Ip, po::value<std::string>()->default_value("127.0.0.1"),     "Local MIHF IP address")
            (odtone::sap::kConf_MIHF_Local_Port, po::value<ushort>()->default_value(1025),         "Local MIHF communication port")
            (odtone::sap::kConf_MIH_SAP_dest, po::value<std::string>()->default_value("mihf2_ue"), "MIHF destination");

        odtone::mih::config cfg(desc);
        cfg.parse(argc, argv, odtone::sap::kConf_File);

        if (cfg.help()) {
            std::cerr << desc << std::endl;
            return EXIT_SUCCESS;
        }

        mih_user usr(cfg, ios);

        ios.run();

    } catch(std::exception& e) {
        log_(0, "exception: ", e.what());
    }
}

// EOF ////////////////////////////////////////////////////////////////////////

