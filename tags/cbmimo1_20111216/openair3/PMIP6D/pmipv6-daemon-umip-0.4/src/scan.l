/*
 * $Id: scan.l 1.44 06/05/12 11:48:36+03:00 vnuorval@tcs.hut.fi $
 *
 * This file is part of the MIPL Mobile IPv6 for Linux.
 *
 * Authors: Antti Tuominen <anttit@tcs.hut.fi>
 *          Ville Nuorvala <vnuorval@tcs.hut.fi>
 *
 * Copyright 2003-2005 Go-Core Project
 * Copyright 2003-2006 Helsinki University of Technology
 *
 * MIPL Mobile IPv6 for Linux is free software; you can redistribute
 * it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; version 2 of
 * the License.
 *
 * MIPL Mobile IPv6 for Linux is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MIPL Mobile IPv6 for Linux; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA
 */
/*
 * This file is part of the PMIP, Proxy Mobile IPv6 for Linux.
 *
 * Authors: OPENAIR3 <openair_tech@eurecom.fr>
 *
 * Copyright 2010-2011 EURECOM (Sophia-Antipolis, FRANCE)
 * 
 * Proxy Mobile IPv6 (or PMIPv6, or PMIP) is a network-based mobility 
 * management protocol standardized by IETF. It is a protocol for building 
 * a common and access technology independent of mobile core networks, 
 * accommodating various access technologies such as WiMAX, 3GPP, 3GPP2 
 * and WLAN based access architectures. Proxy Mobile IPv6 is the only 
 * network-based mobility management protocol standardized by IETF.
 * 
 * PMIP Proxy Mobile IPv6 for Linux has been built above MIPL free software;
 * which it involves that it is under the same terms of GNU General Public
 * License version 2. See MIPL terms condition if you need more details. 
 */

%top{
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
}

%{
#include <arpa/inet.h>
#include "gram.h"

#define YY_NO_UNPUT 1

int lineno = 1;

static int yywrap(void) { return 1; }
void yyerror(char *s);

%}

%option nounput
%option noyyget_lineno noyyset_lineno
%option noyyget_in noyyset_in
%option noyyget_out noyyset_out
%option noyyget_debug noyyset_debug
%option noyyget_leng
%option noyyget_text

ws		[ \t]+
comment		#.*
nl		\n
qstring		\"[^\"\n]*[\"\n]
digit		[0-9]
number		({digit})+
decimal		({number}"."{number})
hexdec		([a-f]|[A-F]|[0-9])
addr1		{hexdec}{1,4}":"({hexdec}{1,4}":")*(":"{hexdec}{1,4})+
addr2		{hexdec}{1,4}(":"{hexdec}{1,4}){0,6}"::"
addr3		({hexdec}{1,4}":"){7}{hexdec}{1,4}
addr		({addr1}|{addr2}|{addr3}|"::")
true		(enabled|allow|true)
false		(disabled|deny|block|false)
%%

{ws}		;
{comment}	;
{nl}		{ lineno++; }
{qstring}	{
			yylval.string = strdup(yytext+1);
			if (yylval.string[yyleng-2] != '"')
				fprintf(stderr, "Unterminated string\n");
			else
				yylval.string[yyleng-2] = '\0';
			return QSTRING;
		}

NodeConfig			{ return MIP6ENTITY; }
HA				{ return MIP6HA; }
CN				{ return MIP6CN; }
MN				{ return MIP6MN; }
LMA				{ return MIP6LMA; }
MAG				{ return MIP6MAG; }
DebugLevel			{ return DEBUGLEVEL; }
DebugLogFile			{ return DEBUGLOGFILE; }
DoRouteOptimizationCN		{ return DOROUTEOPTIMIZATIONCN; }
DoRouteOptimizationMN		{ return DOROUTEOPTIMIZATIONMN; }
HomeAddress			{ return HOMEADDRESS; }
HomeAgentAddress		{ return HOMEAGENTADDRESS; }
InitialBindackTimeoutFirstReg	{ return INITIALBINDACKTIMEOUTFIRSTREG; }
InitialBindackTimeoutReReg	{ return INITIALBINDACKTIMEOUTREREG; }
LinkName			{ return LINKNAME; }
HaMaxBindingLife		{ return HAMAXBINDINGLIFE; }
MnMaxHaBindingLife		{ return MNMAXHABINDINGLIFE; }
MnMaxCnBindingLife		{ return MNMAXCNBINDINGLIFE; }
MaxMobPfxAdvInterval		{ return MAXMOBPFXADVINTERVAL; }
MinMobPfxAdvInterval		{ return MINMOBPFXADVINTERVAL; }
MnHomeLink			{ return MNHOMELINK; }
NonVolatileBindingCache		{ return NONVOLATILEBINDINGCACHE; }
SendMobPfxSols			{ return SENDMOBPFXSOLS; }
SendUnsolMobPfxAdvs	       	{ return SENDUNSOLMOBPFXADVS; }
SendMobPfxAdvs			{ return SENDMOBPFXADVS; }
UseMovementModule		{ return USEMOVEMENTMODULE; }
UsePolicyModule			{ return USEPOLICYMODULE; }
IPsecPolicySet			{ return IPSECPOLICYSET; }
IPsecPolicy			{ return IPSECPOLICY; }
IPsecType			{ return IPSECTYPE; }
UseAltCoa			{ return USEALTCOA; }
UseESP				{ return USEESP; }
UseAH				{ return USEAH; }
UseIPComp			{ return USEIPCOMP; }
UseMnHaIPsec			{ return USEMNHAIPSEC; }
KeyMngMobCapability		{ return KEYMNGMOBCAPABILITY; }
HomeRegBinding			{ return HOMEREGBINDING; }
Mh				{ return MH; }
MobPfxDisc			{ return MOBPFXDISC; }
TunnelHomeTesting		{ return TUNNELHOMETESTING; }
TunnelMh			{ return TUNNELMH; }
TunnelPayload			{ return TUNNELPAYLOAD; }
MnRoPolicy       		{ return MNROPOLICY; }
ICMP				{ return ICMP; }
any				{ return ANY; }
DoRouteOptimization		{ return DOROUTEOPT; }
DefaultBindingAclPolicy		{ return DEFAULTBINDINGACLPOLICY; }
BindingAclPolicy		{ return BINDINGACLPOLICY; }
UseCnBuAck			{ return USECNBUACK; }
Interface			{ return INTERFACE; }
IfName				{ return IFNAME; }
IfType				{ return IFTYPE; }
MnIfPreference			{ return MNIFPREFERENCE; }
MnUseAllInterfaces     		{ return MNUSEALLINTERFACES; }
MnRouterProbes			{ return MNROUTERPROBES; }
MnRouterProbeTimeout		{ return MNROUTERPROBETIMEOUT; }
MnDiscardHaParamProb		{ return MNDISCARDHAPARAMPROB; }
OptimisticHandoff		{ return OPTIMISTICHANDOFF; }
LmaAddress					{ return LMAADDRESS; }
MagAddressIngress			{ return MAGADDRESSINGRESS; }
MagAddressEgress			{ return MAGADDRESSEGRESS; }
OurAddress					{ return OURADDRESS; }
HomeNetworkPrefix			{ return HOMENETWORKPREFIX; }
PBULifeTime					{ return PBULIFETIME; }
PBALifeTime					{ return PBALIFETIME; }
NRetransmissionTime			{ return NRETRANSMISSIONTIME; }
MaxMessageRetransmissions	{ return MAXMESSAGERETRANSMISSIONS; }
TunnelingEnabled			{ return TUNNELINGENABLED; }
DynamicTunnelingEnabled		{ return DYNAMICTUNNELINGENABLED; }
ProxyMipLma					{ return PROXYMIPLMA; }
ProxyMipMag					{ return PROXYMIPMAG; }
RadiusPassword				{ return RADIUSPASSWORD; }
RadiusClientConfigFile		{ return RADIUSCLIENTCONFIGFILE; }

internal			{ return INTERNAL; }

{addr}		{
			int i;

			i = inet_pton(AF_INET6, yytext, &yylval.addr);

			if (i < 1) {
				return INV_TOKEN;
			}
			return ADDR;
		}

{number}	{ yylval.num = atoi(yytext); return NUMBER; }

{decimal}	{ yylval.dec = atof(yytext); return DECIMAL; }

{true}  	{ yylval.bool = 1; return BOOL; }

{false} 	{ yylval.bool = 0; return BOOL; }

"{"|"}"|";"|"/"|"="|"("|")"|","	{ return *yytext; }

.		{ return INV_TOKEN; }
%%
