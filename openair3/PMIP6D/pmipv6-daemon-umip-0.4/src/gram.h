
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     QSTRING = 258,
     ADDR = 259,
     BOOL = 260,
     NUMBER = 261,
     DECIMAL = 262,
     NUMPAIR = 263,
     MIP6ENTITY = 264,
     DEBUGLEVEL = 265,
     DEBUGLOGFILE = 266,
     DOROUTEOPTIMIZATIONCN = 267,
     DOROUTEOPTIMIZATIONMN = 268,
     HOMEADDRESS = 269,
     HOMEAGENTADDRESS = 270,
     INITIALBINDACKTIMEOUTFIRSTREG = 271,
     INITIALBINDACKTIMEOUTREREG = 272,
     LINKNAME = 273,
     HAMAXBINDINGLIFE = 274,
     MNMAXHABINDINGLIFE = 275,
     MNMAXCNBINDINGLIFE = 276,
     MAXMOBPFXADVINTERVAL = 277,
     MINMOBPFXADVINTERVAL = 278,
     MNHOMELINK = 279,
     HAHOMELINK = 280,
     NONVOLATILEBINDINGCACHE = 281,
     SENDMOBPFXSOLS = 282,
     SENDUNSOLMOBPFXADVS = 283,
     SENDMOBPFXADVS = 284,
     IPSECPOLICYSET = 285,
     IPSECPOLICY = 286,
     IPSECTYPE = 287,
     USEALTCOA = 288,
     USEESP = 289,
     USEAH = 290,
     USEIPCOMP = 291,
     BLOCK = 292,
     USEMNHAIPSEC = 293,
     KEYMNGMOBCAPABILITY = 294,
     HOMEREGBINDING = 295,
     MH = 296,
     MOBPFXDISC = 297,
     TUNNELHOMETESTING = 298,
     TUNNELMH = 299,
     TUNNELPAYLOAD = 300,
     USEMOVEMENTMODULE = 301,
     USEPOLICYMODULE = 302,
     MIP6CN = 303,
     MIP6MN = 304,
     MIP6HA = 305,
     INTERNAL = 306,
     MNROPOLICY = 307,
     ICMP = 308,
     ANY = 309,
     DOROUTEOPT = 310,
     DEFAULTBINDINGACLPOLICY = 311,
     BINDINGACLPOLICY = 312,
     MNADDRESS = 313,
     USECNBUACK = 314,
     INTERFACE = 315,
     IFNAME = 316,
     IFTYPE = 317,
     MNIFPREFERENCE = 318,
     MNUSEALLINTERFACES = 319,
     MNROUTERPROBES = 320,
     MNROUTERPROBETIMEOUT = 321,
     MNDISCARDHAPARAMPROB = 322,
     OPTIMISTICHANDOFF = 323,
     MIP6LMA = 324,
     MIP6MAG = 325,
     PROXYMIPLMA = 326,
     PROXYMIPMAG = 327,
     ALLLMAMULTICASTADDRESS = 328,
     LMAADDRESS = 329,
     MAGADDRESSINGRESS = 330,
     MAGADDRESSEGRESS = 331,
     OURADDRESS = 332,
     HOMENETWORKPREFIX = 333,
     PBULIFETIME = 334,
     PBALIFETIME = 335,
     NRETRANSMISSIONTIME = 336,
     MAXMESSAGERETRANSMISSIONS = 337,
     TUNNELINGENABLED = 338,
     DYNAMICTUNNELINGENABLED = 339,
     RADIUSPASSWORD = 340,
     RADIUSCLIENTCONFIGFILE = 341,
     INV_TOKEN = 342
   };
#endif
/* Tokens.  */
#define QSTRING 258
#define ADDR 259
#define BOOL 260
#define NUMBER 261
#define DECIMAL 262
#define NUMPAIR 263
#define MIP6ENTITY 264
#define DEBUGLEVEL 265
#define DEBUGLOGFILE 266
#define DOROUTEOPTIMIZATIONCN 267
#define DOROUTEOPTIMIZATIONMN 268
#define HOMEADDRESS 269
#define HOMEAGENTADDRESS 270
#define INITIALBINDACKTIMEOUTFIRSTREG 271
#define INITIALBINDACKTIMEOUTREREG 272
#define LINKNAME 273
#define HAMAXBINDINGLIFE 274
#define MNMAXHABINDINGLIFE 275
#define MNMAXCNBINDINGLIFE 276
#define MAXMOBPFXADVINTERVAL 277
#define MINMOBPFXADVINTERVAL 278
#define MNHOMELINK 279
#define HAHOMELINK 280
#define NONVOLATILEBINDINGCACHE 281
#define SENDMOBPFXSOLS 282
#define SENDUNSOLMOBPFXADVS 283
#define SENDMOBPFXADVS 284
#define IPSECPOLICYSET 285
#define IPSECPOLICY 286
#define IPSECTYPE 287
#define USEALTCOA 288
#define USEESP 289
#define USEAH 290
#define USEIPCOMP 291
#define BLOCK 292
#define USEMNHAIPSEC 293
#define KEYMNGMOBCAPABILITY 294
#define HOMEREGBINDING 295
#define MH 296
#define MOBPFXDISC 297
#define TUNNELHOMETESTING 298
#define TUNNELMH 299
#define TUNNELPAYLOAD 300
#define USEMOVEMENTMODULE 301
#define USEPOLICYMODULE 302
#define MIP6CN 303
#define MIP6MN 304
#define MIP6HA 305
#define INTERNAL 306
#define MNROPOLICY 307
#define ICMP 308
#define ANY 309
#define DOROUTEOPT 310
#define DEFAULTBINDINGACLPOLICY 311
#define BINDINGACLPOLICY 312
#define MNADDRESS 313
#define USECNBUACK 314
#define INTERFACE 315
#define IFNAME 316
#define IFTYPE 317
#define MNIFPREFERENCE 318
#define MNUSEALLINTERFACES 319
#define MNROUTERPROBES 320
#define MNROUTERPROBETIMEOUT 321
#define MNDISCARDHAPARAMPROB 322
#define OPTIMISTICHANDOFF 323
#define MIP6LMA 324
#define MIP6MAG 325
#define PROXYMIPLMA 326
#define PROXYMIPMAG 327
#define ALLLMAMULTICASTADDRESS 328
#define LMAADDRESS 329
#define MAGADDRESSINGRESS 330
#define MAGADDRESSEGRESS 331
#define OURADDRESS 332
#define HOMENETWORKPREFIX 333
#define PBULIFETIME 334
#define PBALIFETIME 335
#define NRETRANSMISSIONTIME 336
#define MAXMESSAGERETRANSMISSIONS 337
#define TUNNELINGENABLED 338
#define DYNAMICTUNNELINGENABLED 339
#define RADIUSPASSWORD 340
#define RADIUSCLIENTCONFIGFILE 341
#define INV_TOKEN 342




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 93 "gram.y"

	char *string;
	struct in6_addr addr;
	char bool;
	unsigned int num;
	unsigned int numpair[2];
	double dec;



/* Line 1676 of yacc.c  */
#line 237 "gram.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


