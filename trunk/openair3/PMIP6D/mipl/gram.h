/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

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
     DOROUTEOPTIMIZATIONCN = 266,
     DOROUTEOPTIMIZATIONMN = 267,
     HOMEADDRESS = 268,
     HOMEAGENTADDRESS = 269,
     INITIALBINDACKTIMEOUTFIRSTREG = 270,
     INITIALBINDACKTIMEOUTREREG = 271,
     LINKNAME = 272,
     HAMAXBINDINGLIFE = 273,
     MNMAXHABINDINGLIFE = 274,
     MNMAXCNBINDINGLIFE = 275,
     MAXMOBPFXADVINTERVAL = 276,
     MINMOBPFXADVINTERVAL = 277,
     MNHOMELINK = 278,
     HAHOMELINK = 279,
     NONVOLATILEBINDINGCACHE = 280,
     SENDMOBPFXSOLS = 281,
     SENDUNSOLMOBPFXADVS = 282,
     SENDMOBPFXADVS = 283,
     IPSECPOLICYSET = 284,
     IPSECPOLICY = 285,
     IPSECTYPE = 286,
     USEALTCOA = 287,
     USEESP = 288,
     USEAH = 289,
     USEIPCOMP = 290,
     BLOCK = 291,
     USEMNHAIPSEC = 292,
     KEYMNGMOBCAPABILITY = 293,
     HOMEREGBINDING = 294,
     MH = 295,
     MOBPFXDISC = 296,
     TUNNELHOMETESTING = 297,
     TUNNELMH = 298,
     TUNNELPAYLOAD = 299,
     USEMOVEMENTMODULE = 300,
     USEPOLICYMODULE = 301,
     MIP6CN = 302,
     MIP6MN = 303,
     MIP6HA = 304,
     INTERNAL = 305,
     MNROPOLICY = 306,
     ICMP = 307,
     ANY = 308,
     DOROUTEOPT = 309,
     DEFAULTBINDINGACLPOLICY = 310,
     BINDINGACLPOLICY = 311,
     MNADDRESS = 312,
     USECNBUACK = 313,
     INTERFACE = 314,
     IFNAME = 315,
     IFTYPE = 316,
     MNIFPREFERENCE = 317,
     MNUSEALLINTERFACES = 318,
     MNROUTERPROBES = 319,
     MNROUTERPROBETIMEOUT = 320,
     MNDISCARDHAPARAMPROB = 321,
     OPTIMISTICHANDOFF = 322,
     INV_TOKEN = 323
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
#define DOROUTEOPTIMIZATIONCN 266
#define DOROUTEOPTIMIZATIONMN 267
#define HOMEADDRESS 268
#define HOMEAGENTADDRESS 269
#define INITIALBINDACKTIMEOUTFIRSTREG 270
#define INITIALBINDACKTIMEOUTREREG 271
#define LINKNAME 272
#define HAMAXBINDINGLIFE 273
#define MNMAXHABINDINGLIFE 274
#define MNMAXCNBINDINGLIFE 275
#define MAXMOBPFXADVINTERVAL 276
#define MINMOBPFXADVINTERVAL 277
#define MNHOMELINK 278
#define HAHOMELINK 279
#define NONVOLATILEBINDINGCACHE 280
#define SENDMOBPFXSOLS 281
#define SENDUNSOLMOBPFXADVS 282
#define SENDMOBPFXADVS 283
#define IPSECPOLICYSET 284
#define IPSECPOLICY 285
#define IPSECTYPE 286
#define USEALTCOA 287
#define USEESP 288
#define USEAH 289
#define USEIPCOMP 290
#define BLOCK 291
#define USEMNHAIPSEC 292
#define KEYMNGMOBCAPABILITY 293
#define HOMEREGBINDING 294
#define MH 295
#define MOBPFXDISC 296
#define TUNNELHOMETESTING 297
#define TUNNELMH 298
#define TUNNELPAYLOAD 299
#define USEMOVEMENTMODULE 300
#define USEPOLICYMODULE 301
#define MIP6CN 302
#define MIP6MN 303
#define MIP6HA 304
#define INTERNAL 305
#define MNROPOLICY 306
#define ICMP 307
#define ANY 308
#define DOROUTEOPT 309
#define DEFAULTBINDINGACLPOLICY 310
#define BINDINGACLPOLICY 311
#define MNADDRESS 312
#define USECNBUACK 313
#define INTERFACE 314
#define IFNAME 315
#define IFTYPE 316
#define MNIFPREFERENCE 317
#define MNUSEALLINTERFACES 318
#define MNROUTERPROBES 319
#define MNROUTERPROBETIMEOUT 320
#define MNDISCARDHAPARAMPROB 321
#define OPTIMISTICHANDOFF 322
#define INV_TOKEN 323




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 92 "gram.y"
typedef union YYSTYPE {
	char *string;
	struct in6_addr addr;
	char bool;
	unsigned int num;
	unsigned int numpair[2];
	double dec;
} YYSTYPE;
/* Line 1447 of yacc.c.  */
#line 183 "gram.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



