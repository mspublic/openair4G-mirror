/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file otg_config.h main used structures
* \brief otg structure 
* \author A. Hafsaoui
* \date 2011
* \version 0.1
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning

*/

#ifndef __OTG_CONFIG_H__
#	define __OTG_CONFIG_H__


/*!\brief It indicates that: the payload and the transport header (not otg header) are copied from static strings (HEADER_STRING and PAYLOAD_STRING) */
#define STRING_STATIC 1

/*!\brief Define the number of states*/
#define MAX_NUM_TRAFFIC_STATE 2 //  we have two states: STATE 0 and STATE 1


#define OTG_FLAG_SIZE 3
#define OTG_FLAG "OTG"

#define IDT_TH 100 
#define SIZE_COEF 1000
#define MAXIDT_TIME 1000000 //(1 second = 1000000 microseconds)


/*!\brief Define the max size of the TX buffer */ 
#define MAX_BUFF_TX 10000
 
/*!\brief Define the min size of the payload*/ 
#define PAYLOAD_MIN 1

/*!\brief Define the min size of the payload, according to the Jumbo frame*/ 
#ifdef JUMBO_FRAME
 	#define PAYLOAD_MAX 9000
#else
	#define PAYLOAD_MAX 1500
#endif


/*!\brief Define the size of IP version header, in bytes*/
#define HDR_IP_v4_MIN 20
#define HDR_IP_v4_MAX 60
#define HDR_IP_v6 60

/*!\brief Define the size of TCP header, in bytes*/
#define HDR_TCP 20

/*!\brief Define the size of UDP header, in bytes*/
#define HDR_UDP 8


/*!\brief Define the max size of OTG header*/
#define SIZE_OTG_HEADER 8


/*!\brief Define the Alphabet string to generate the payload of the packet*/
#define	ALPHABET_NUM_LETTER "abcdefghijklmnopqrstuvwyzABCDEFGHIGKLMNOPQRSTUVWXYZ0123456789"

/*!\brief Define the Alphabet string to generate the header of the packet*/
#define	ALPHABET_NUM "0123456789"

/*!\brief Define the static string to generate the header of the packet*/
#define HEADER_STRING "5048717272261061594909177115977673656394812939088509638561159848103044447631759621785741859753883189"

/*!\brief Define the static string to generate the payload of the packet*/
#define PAYLOAD_STRING "UVk5miARQfZGDFKf1wS0dt57kHigd0fXNrUZCjIhpyOS4pZWMHOP1GPdgXmlPtarLUjd3Rmkg05bhUZWtDDmdhrl5EzMZz6DkhIg0Uq7NlaU8ZGrt9EzgVLdr9SiBOLLXiTN3aMInMrlDYFYZ8n5WYbfZTnpz13lbMY4OBE4eWfIMLvBLLyzzzEqjUGILBVMfKGVccPi0VSCyg28RqAiR3z1P6zryk4FWFp0G78AUT1hZWhGcGOTDcKj9bCzny592m1Dj123KWczIm5KVLupO7AP83flqamimfLz6GtHrz5ZN2BAEVQjUhYSc35s5jDhofIlL2U4qPT3Ilsd7amTjaCl5zE0L89ZeIcPCWKSEuNdH5gG8sojuSvph1hU0gG4QOLhCk15IE8eCeMCz2LTL68U0hEQqeM6UmgmA9j7Eid7oPzQHbzj8A30HzGXGhWpt4CT3MSwWVvcCWSbYjkYGgOhHj5csTsONWyGAh5l3qquf8v3jGRSRu0nGXqYILCkw1SX9Na46qodrN6BnPl49djH2AuAaYKAStoR9oL7I1aZG6rVLFPMIZiAqF1tuDVcX9VWnyTVpTMXR6GtBp5bgfDyKuT4ZE9MDUASikGA5hoMfX5Gf2Ml7eLGBtEqZF4rouczHI0DRfgX4ev967n6dYFFkaXbFTvWdykN5bfMinzcrWeqVrmZhTvtUkvq3Rc9enM9qTNz6cDo0HHM0VD8EYtpaPH3yG2CYGDgogHlkaCcHaOyViyq8RH8wf4WQWoHuTNG1kWdkpgTrWic5Gv5p24O9YAPMOn6A1IsdvwpOF85qj8nPvj4nfIo385HOjGfadzfBXueruaKEa0lvbhLgS1bQWKv5fE7k2cMPzQ8USIpUyBhBGUHsLKaykvsr1qDTueAUWAGH8VqyozZZkyhWahjmFEEwU6hhcK1Z9wv9jOAAeqopQvbQFm4aQzzBwGIAhBqhZMiarIBwYPOFdPmK1hKHIa94GGtQbMZ0n83IGt6w8K3dqfOhmpQWqSRZscFwPuo4uhC0ByoC9hFpnizCBfoRZ7Gj9cGOzVLT2eMtD0XC8rUnDiR3p7Ke4ho6lWMLHmtCr7VWYIpY19dtiWoyU0FQ7VMlHoWeBhIUfuG54WVVX0h5Mvvvo0cnLQzh4knysVhAfQw9EhXq94mLrI9GydUaTihOvydQikfq2CrvLeNK81msBlYYoK0aT7OewTUI51YYufj7kYGkPVDf7t5n3VnMV3ShMERKwFyTNHQZyo9ccFibYdoT1FyMAfVeMDO89bUMKAD7RFaT9kUZpaIiH5W7dIbPcPPdSBSr1krKPtuQEeHVgf4OcQLfpOWtsEya4ftXpNw76RPCXmp4rjKt1mCh0pBiTYkQ5GDnj2khLZMzb1uua6R1ika8ACglrs1n0vDbnNjZEVpIMK4OGLFOXIOn9UBserI4Pa63PhUl49TGLNjiqQWdnAsolTKrcjnSklN1swcmyVU8B5gTO4Y3vhkG2U2"


#endif
