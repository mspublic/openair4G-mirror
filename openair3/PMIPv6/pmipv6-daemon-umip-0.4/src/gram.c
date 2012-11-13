
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 46 "gram.y"


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <pthread.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <netinet/ip6mh.h>
#include "mipv6.h"
#include "ha.h"
#include "mn.h"
#include "conf.h"
#include "policy.h"
#include "xfrm.h"
#include "prefix.h"
#include "util.h"
#include "ipsec.h"
#include "rtnl.h"

struct net_iface ni = {
	.mip6_if_entity = MIP6_ENTITY_NO,
	.mn_if_preference = POL_MN_IF_DEF_PREFERENCE,
};

struct home_addr_info hai = {
	.ro_policies = LIST_HEAD_INIT(hai.ro_policies)
};

struct policy_bind_acl_entry *bae = NULL;

struct ipsec_policy_set {
	struct in6_addr ha;
	struct list_head hoa_list;
};

struct ipsec_policy_set ipsec_ps = {
	.hoa_list = LIST_HEAD_INIT(ipsec_ps.hoa_list)
};


extern int lineno;
extern char *yytext;

static void yyerror(char *s) {
	fprintf(stderr, "Error in configuration file %s\n", conf.config_file);
	fprintf(stderr, "line %d: %s at '%s'\n", lineno, s, yytext);
}

static void uerror(const char *fmt, ...) {
	char s[1024];
	va_list args;

	fprintf(stderr, "Error in configuration file %s\n", conf.config_file);
	va_start(args, fmt);
	vsprintf(s, fmt, args);
	fprintf(stderr, "line %d: %s\n", lineno, s);
	va_end(args);
}



/* Line 189 of yacc.c  */
#line 139 "gram.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     QSTRING = 258,
     ADDR = 259,
     MACADDR = 260,
     BOOL = 261,
     NUMBER = 262,
     DECIMAL = 263,
     NUMPAIR = 264,
     MIP6ENTITY = 265,
     DEBUGLEVEL = 266,
     DEBUGLOGFILE = 267,
     DOROUTEOPTIMIZATIONCN = 268,
     DOROUTEOPTIMIZATIONMN = 269,
     HOMEADDRESS = 270,
     HOMEAGENTADDRESS = 271,
     INITIALBINDACKTIMEOUTFIRSTREG = 272,
     INITIALBINDACKTIMEOUTREREG = 273,
     LINKNAME = 274,
     HAMAXBINDINGLIFE = 275,
     MNMAXHABINDINGLIFE = 276,
     MNMAXCNBINDINGLIFE = 277,
     MAXMOBPFXADVINTERVAL = 278,
     MINMOBPFXADVINTERVAL = 279,
     MNHOMELINK = 280,
     HAHOMELINK = 281,
     NONVOLATILEBINDINGCACHE = 282,
     SENDMOBPFXSOLS = 283,
     SENDUNSOLMOBPFXADVS = 284,
     SENDMOBPFXADVS = 285,
     IPSECPOLICYSET = 286,
     IPSECPOLICY = 287,
     IPSECTYPE = 288,
     USEALTCOA = 289,
     USEESP = 290,
     USEAH = 291,
     USEIPCOMP = 292,
     BLOCK = 293,
     USEMNHAIPSEC = 294,
     KEYMNGMOBCAPABILITY = 295,
     HOMEREGBINDING = 296,
     MH = 297,
     MOBPFXDISC = 298,
     TUNNELHOMETESTING = 299,
     TUNNELMH = 300,
     TUNNELPAYLOAD = 301,
     USEMOVEMENTMODULE = 302,
     USEPOLICYMODULE = 303,
     MIP6CN = 304,
     MIP6MN = 305,
     MIP6HA = 306,
     INTERNAL = 307,
     MNROPOLICY = 308,
     ICMP = 309,
     ANY = 310,
     DOROUTEOPT = 311,
     DEFAULTBINDINGACLPOLICY = 312,
     BINDINGACLPOLICY = 313,
     MNADDRESS = 314,
     USECNBUACK = 315,
     INTERFACE = 316,
     IFNAME = 317,
     IFTYPE = 318,
     MNIFPREFERENCE = 319,
     MNUSEALLINTERFACES = 320,
     MNROUTERPROBES = 321,
     MNROUTERPROBETIMEOUT = 322,
     MNDISCARDHAPARAMPROB = 323,
     OPTIMISTICHANDOFF = 324,
     RFC5213TIMESTAMPBASEDAPPROACHINUSE = 325,
     RFC5213MOBILENODEGENERATEDTIMESTAMPINUSE = 326,
     RFC5213FIXEDMAGLINKLOCALADDRESSONALLACCESSLINKS = 327,
     RFC5213FIXEDMAGLINKLAYERADDRESSONALLACCESSLINKS = 328,
     RFC5213MINDELAYBEFOREBCEDELETE = 329,
     RFC5213MAXDELAYBEFORENEWBCEASSIGN = 330,
     RFC5213TIMESTAMPVALIDITYWINDOW = 331,
     RFC5213ENABLEMAGLOCALROUTING = 332,
     MIP6LMA = 333,
     MIP6MAG = 334,
     PROXYMIPLMA = 335,
     PROXYMIPMAG = 336,
     ALLLMAMULTICASTADDRESS = 337,
     LMAADDRESS = 338,
     LMAPMIPNETWORKDEVICE = 339,
     LMACORENETWORKADDRESS = 340,
     LMACORENETWORKDEVICE = 341,
     MAGADDRESSINGRESS = 342,
     MAGADDRESSEGRESS = 343,
     MAG1ADDRESSINGRESS = 344,
     MAG1ADDRESSEGRESS = 345,
     MAG2ADDRESSINGRESS = 346,
     MAG2ADDRESSEGRESS = 347,
     MAG3ADDRESSINGRESS = 348,
     MAG3ADDRESSEGRESS = 349,
     MAGDEVICEINGRESS = 350,
     MAGDEVICEEGRESS = 351,
     OURADDRESS = 352,
     HOMENETWORKPREFIX = 353,
     PBULIFETIME = 354,
     PBALIFETIME = 355,
     RETRANSMISSIONTIMEOUT = 356,
     MAXMESSAGERETRANSMISSIONS = 357,
     TUNNELINGENABLED = 358,
     DYNAMICTUNNELINGENABLED = 359,
     RADIUSPASSWORD = 360,
     RADIUSCLIENTCONFIGFILE = 361,
     PCAPSYSLOGASSOCIATIONGREPSTRING = 362,
     PCAPSYSLOGDEASSOCIATIONGREPSTRING = 363,
     INV_TOKEN = 364
   };
#endif
/* Tokens.  */
#define QSTRING 258
#define ADDR 259
#define MACADDR 260
#define BOOL 261
#define NUMBER 262
#define DECIMAL 263
#define NUMPAIR 264
#define MIP6ENTITY 265
#define DEBUGLEVEL 266
#define DEBUGLOGFILE 267
#define DOROUTEOPTIMIZATIONCN 268
#define DOROUTEOPTIMIZATIONMN 269
#define HOMEADDRESS 270
#define HOMEAGENTADDRESS 271
#define INITIALBINDACKTIMEOUTFIRSTREG 272
#define INITIALBINDACKTIMEOUTREREG 273
#define LINKNAME 274
#define HAMAXBINDINGLIFE 275
#define MNMAXHABINDINGLIFE 276
#define MNMAXCNBINDINGLIFE 277
#define MAXMOBPFXADVINTERVAL 278
#define MINMOBPFXADVINTERVAL 279
#define MNHOMELINK 280
#define HAHOMELINK 281
#define NONVOLATILEBINDINGCACHE 282
#define SENDMOBPFXSOLS 283
#define SENDUNSOLMOBPFXADVS 284
#define SENDMOBPFXADVS 285
#define IPSECPOLICYSET 286
#define IPSECPOLICY 287
#define IPSECTYPE 288
#define USEALTCOA 289
#define USEESP 290
#define USEAH 291
#define USEIPCOMP 292
#define BLOCK 293
#define USEMNHAIPSEC 294
#define KEYMNGMOBCAPABILITY 295
#define HOMEREGBINDING 296
#define MH 297
#define MOBPFXDISC 298
#define TUNNELHOMETESTING 299
#define TUNNELMH 300
#define TUNNELPAYLOAD 301
#define USEMOVEMENTMODULE 302
#define USEPOLICYMODULE 303
#define MIP6CN 304
#define MIP6MN 305
#define MIP6HA 306
#define INTERNAL 307
#define MNROPOLICY 308
#define ICMP 309
#define ANY 310
#define DOROUTEOPT 311
#define DEFAULTBINDINGACLPOLICY 312
#define BINDINGACLPOLICY 313
#define MNADDRESS 314
#define USECNBUACK 315
#define INTERFACE 316
#define IFNAME 317
#define IFTYPE 318
#define MNIFPREFERENCE 319
#define MNUSEALLINTERFACES 320
#define MNROUTERPROBES 321
#define MNROUTERPROBETIMEOUT 322
#define MNDISCARDHAPARAMPROB 323
#define OPTIMISTICHANDOFF 324
#define RFC5213TIMESTAMPBASEDAPPROACHINUSE 325
#define RFC5213MOBILENODEGENERATEDTIMESTAMPINUSE 326
#define RFC5213FIXEDMAGLINKLOCALADDRESSONALLACCESSLINKS 327
#define RFC5213FIXEDMAGLINKLAYERADDRESSONALLACCESSLINKS 328
#define RFC5213MINDELAYBEFOREBCEDELETE 329
#define RFC5213MAXDELAYBEFORENEWBCEASSIGN 330
#define RFC5213TIMESTAMPVALIDITYWINDOW 331
#define RFC5213ENABLEMAGLOCALROUTING 332
#define MIP6LMA 333
#define MIP6MAG 334
#define PROXYMIPLMA 335
#define PROXYMIPMAG 336
#define ALLLMAMULTICASTADDRESS 337
#define LMAADDRESS 338
#define LMAPMIPNETWORKDEVICE 339
#define LMACORENETWORKADDRESS 340
#define LMACORENETWORKDEVICE 341
#define MAGADDRESSINGRESS 342
#define MAGADDRESSEGRESS 343
#define MAG1ADDRESSINGRESS 344
#define MAG1ADDRESSEGRESS 345
#define MAG2ADDRESSINGRESS 346
#define MAG2ADDRESSEGRESS 347
#define MAG3ADDRESSINGRESS 348
#define MAG3ADDRESSEGRESS 349
#define MAGDEVICEINGRESS 350
#define MAGDEVICEEGRESS 351
#define OURADDRESS 352
#define HOMENETWORKPREFIX 353
#define PBULIFETIME 354
#define PBALIFETIME 355
#define RETRANSMISSIONTIMEOUT 356
#define MAXMESSAGERETRANSMISSIONS 357
#define TUNNELINGENABLED 358
#define DYNAMICTUNNELINGENABLED 359
#define RADIUSPASSWORD 360
#define RADIUSCLIENTCONFIGFILE 361
#define PCAPSYSLOGASSOCIATIONGREPSTRING 362
#define PCAPSYSLOGDEASSOCIATIONGREPSTRING 363
#define INV_TOKEN 364




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 111 "gram.y"

	char *string;
	struct in6_addr addr;
	struct in6_addr macaddr;
	char bool;
	unsigned int num;
	unsigned int numpair[2];
	double dec;



/* Line 214 of yacc.c  */
#line 405 "gram.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 417 "gram.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  84
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   427

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  114
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  40
/* YYNRULES -- Number of rules.  */
#define YYNRULES  153
/* YYNRULES -- Number of states.  */
#define YYNSTATES  349

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   364

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   113,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   110,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   111,     2,   112,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    12,    16,    20,    24,    27,
      31,    35,    39,    43,    47,    51,    55,    59,    63,    67,
      71,    75,    78,    82,    86,    90,    94,    98,   102,   106,
     111,   115,   119,   123,   127,   131,   134,   137,   139,   141,
     143,   145,   147,   150,   154,   156,   158,   161,   165,   169,
     174,   176,   179,   183,   189,   193,   197,   201,   205,   207,
     210,   214,   218,   220,   223,   229,   232,   234,   236,   238,
     240,   242,   244,   246,   248,   249,   251,   254,   258,   260,
     262,   264,   265,   267,   270,   271,   273,   276,   277,   279,
     281,   283,   285,   287,   289,   291,   294,   296,   299,   303,
     305,   307,   310,   314,   318,   322,   326,   330,   334,   338,
     342,   346,   350,   354,   358,   362,   366,   370,   374,   378,
     382,   386,   390,   394,   398,   402,   406,   410,   413,   417,
     419,   421,   424,   428,   432,   436,   440,   444,   448,   452,
     456,   460,   464,   468,   472,   476,   480,   484,   488,   492,
     496,   500,   504,   508
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     115,     0,    -1,   116,    -1,   115,   116,    -1,    10,   117,
     110,    -1,    11,     7,   110,    -1,    12,     3,   110,    -1,
      27,     6,   110,    -1,    61,   118,    -1,    28,     6,   110,
      -1,    29,     6,   110,    -1,    30,     6,   110,    -1,    23,
       7,   110,    -1,    24,     7,   110,    -1,    13,     6,   110,
      -1,    14,     6,   110,    -1,    20,     7,   110,    -1,    21,
       7,   110,    -1,    22,     7,   110,    -1,    17,     8,   110,
      -1,    18,     8,   110,    -1,    25,   122,    -1,    39,     6,
     110,    -1,    40,     6,   110,    -1,    47,   141,   110,    -1,
      48,   142,   110,    -1,    57,   143,   110,    -1,    58,   144,
     110,    -1,    60,     6,   110,    -1,    31,   111,   125,   112,
      -1,    65,     6,   110,    -1,    66,     7,   110,    -1,    67,
       8,   110,    -1,    68,     6,   110,    -1,    69,     6,   110,
      -1,    80,   146,    -1,    81,   150,    -1,    49,    -1,    50,
      -1,    51,    -1,    78,    -1,    79,    -1,     3,   119,    -1,
     111,   120,   112,    -1,   110,    -1,   121,    -1,   120,   121,
      -1,    63,   117,   110,    -1,    64,     7,   110,    -1,     3,
     111,   123,   112,    -1,   124,    -1,   123,   124,    -1,    16,
       4,   110,    -1,    15,     4,   113,   145,   110,    -1,    34,
       6,   110,    -1,    53,   138,   110,    -1,   126,   127,   130,
      -1,    16,     4,   110,    -1,   128,    -1,   127,   128,    -1,
      15,   129,   110,    -1,     4,   113,   145,    -1,   131,    -1,
     130,   131,    -1,   132,   134,   136,   137,   110,    -1,    32,
     133,    -1,    41,    -1,    42,    -1,    43,    -1,    44,    -1,
      45,    -1,    46,    -1,    54,    -1,    55,    -1,    -1,   135,
      -1,   135,   135,    -1,   135,   135,   135,    -1,    35,    -1,
      36,    -1,    37,    -1,    -1,     7,    -1,     7,     7,    -1,
      -1,     6,    -1,   139,   140,    -1,    -1,     4,    -1,     6,
      -1,    52,    -1,     3,    -1,     3,    -1,     6,    -1,     7,
      -1,     4,   143,    -1,     7,    -1,     3,   147,    -1,   111,
     148,   112,    -1,   110,    -1,   149,    -1,   148,   149,    -1,
      83,     4,   110,    -1,    84,     3,   110,    -1,    85,     4,
     110,    -1,    86,     3,   110,    -1,    70,     6,   110,    -1,
      71,     6,   110,    -1,    72,     4,   110,    -1,    73,     5,
     110,    -1,    74,     7,   110,    -1,    75,     7,   110,    -1,
      76,     7,   110,    -1,    97,     4,   110,    -1,    98,     4,
     110,    -1,   103,     6,   110,    -1,   104,     6,   110,    -1,
      99,     7,   110,    -1,   100,     7,   110,    -1,   101,     7,
     110,    -1,   102,     7,   110,    -1,    89,     4,   110,    -1,
      90,     4,   110,    -1,    91,     4,   110,    -1,    92,     4,
     110,    -1,    93,     4,   110,    -1,    94,     4,   110,    -1,
       3,   151,    -1,   111,   152,   112,    -1,   110,    -1,   153,
      -1,   152,   153,    -1,    83,     4,   110,    -1,    70,     6,
     110,    -1,    71,     6,   110,    -1,    72,     4,   110,    -1,
      73,     5,   110,    -1,    77,     6,   110,    -1,    97,     4,
     110,    -1,    87,     4,   110,    -1,    88,     4,   110,    -1,
      95,     3,   110,    -1,    96,     3,   110,    -1,    98,     4,
     110,    -1,   103,     6,   110,    -1,   104,     6,   110,    -1,
      99,     7,   110,    -1,   100,     7,   110,    -1,   101,     7,
     110,    -1,   102,     7,   110,    -1,   105,     3,   110,    -1,
     106,     3,   110,    -1,   107,     3,   110,    -1,   108,     3,
     110,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   247,   247,   248,   251,   255,   259,   263,   267,   268,
     272,   276,   280,   284,   288,   292,   296,   305,   314,   323,
     327,   331,   332,   336,   340,   341,   342,   346,   350,   354,
     355,   359,   363,   368,   372,   376,   377,   380,   381,   382,
     383,   384,   387,   413,   414,   417,   418,   421,   425,   431,
     475,   476,   479,   483,   488,   492,   495,   518,   524,   525,
     528,   531,   547,   548,   551,   619,   622,   623,   624,   625,
     626,   627,   628,   629,   633,   637,   638,   639,   642,   643,
     644,   647,   648,   649,   652,   653,   656,   671,   672,   675,
     678,   682,   688,   697,   704,   707,   722,   733,   740,   741,
     744,   745,   748,   752,   756,   760,   764,   768,   772,   776,
     780,   787,   794,   801,   805,   809,   813,   817,   824,   831,
     838,   842,   846,   850,   854,   858,   862,   868,   875,   876,
     879,   880,   883,   887,   891,   895,   899,   903,   907,   911,
     915,   919,   923,   927,   931,   935,   939,   946,   953,   960,
     964,   968,   972,   976
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "QSTRING", "ADDR", "MACADDR", "BOOL",
  "NUMBER", "DECIMAL", "NUMPAIR", "MIP6ENTITY", "DEBUGLEVEL",
  "DEBUGLOGFILE", "DOROUTEOPTIMIZATIONCN", "DOROUTEOPTIMIZATIONMN",
  "HOMEADDRESS", "HOMEAGENTADDRESS", "INITIALBINDACKTIMEOUTFIRSTREG",
  "INITIALBINDACKTIMEOUTREREG", "LINKNAME", "HAMAXBINDINGLIFE",
  "MNMAXHABINDINGLIFE", "MNMAXCNBINDINGLIFE", "MAXMOBPFXADVINTERVAL",
  "MINMOBPFXADVINTERVAL", "MNHOMELINK", "HAHOMELINK",
  "NONVOLATILEBINDINGCACHE", "SENDMOBPFXSOLS", "SENDUNSOLMOBPFXADVS",
  "SENDMOBPFXADVS", "IPSECPOLICYSET", "IPSECPOLICY", "IPSECTYPE",
  "USEALTCOA", "USEESP", "USEAH", "USEIPCOMP", "BLOCK", "USEMNHAIPSEC",
  "KEYMNGMOBCAPABILITY", "HOMEREGBINDING", "MH", "MOBPFXDISC",
  "TUNNELHOMETESTING", "TUNNELMH", "TUNNELPAYLOAD", "USEMOVEMENTMODULE",
  "USEPOLICYMODULE", "MIP6CN", "MIP6MN", "MIP6HA", "INTERNAL",
  "MNROPOLICY", "ICMP", "ANY", "DOROUTEOPT", "DEFAULTBINDINGACLPOLICY",
  "BINDINGACLPOLICY", "MNADDRESS", "USECNBUACK", "INTERFACE", "IFNAME",
  "IFTYPE", "MNIFPREFERENCE", "MNUSEALLINTERFACES", "MNROUTERPROBES",
  "MNROUTERPROBETIMEOUT", "MNDISCARDHAPARAMPROB", "OPTIMISTICHANDOFF",
  "RFC5213TIMESTAMPBASEDAPPROACHINUSE",
  "RFC5213MOBILENODEGENERATEDTIMESTAMPINUSE",
  "RFC5213FIXEDMAGLINKLOCALADDRESSONALLACCESSLINKS",
  "RFC5213FIXEDMAGLINKLAYERADDRESSONALLACCESSLINKS",
  "RFC5213MINDELAYBEFOREBCEDELETE", "RFC5213MAXDELAYBEFORENEWBCEASSIGN",
  "RFC5213TIMESTAMPVALIDITYWINDOW", "RFC5213ENABLEMAGLOCALROUTING",
  "MIP6LMA", "MIP6MAG", "PROXYMIPLMA", "PROXYMIPMAG",
  "ALLLMAMULTICASTADDRESS", "LMAADDRESS", "LMAPMIPNETWORKDEVICE",
  "LMACORENETWORKADDRESS", "LMACORENETWORKDEVICE", "MAGADDRESSINGRESS",
  "MAGADDRESSEGRESS", "MAG1ADDRESSINGRESS", "MAG1ADDRESSEGRESS",
  "MAG2ADDRESSINGRESS", "MAG2ADDRESSEGRESS", "MAG3ADDRESSINGRESS",
  "MAG3ADDRESSEGRESS", "MAGDEVICEINGRESS", "MAGDEVICEEGRESS", "OURADDRESS",
  "HOMENETWORKPREFIX", "PBULIFETIME", "PBALIFETIME",
  "RETRANSMISSIONTIMEOUT", "MAXMESSAGERETRANSMISSIONS", "TUNNELINGENABLED",
  "DYNAMICTUNNELINGENABLED", "RADIUSPASSWORD", "RADIUSCLIENTCONFIGFILE",
  "PCAPSYSLOGASSOCIATIONGREPSTRING", "PCAPSYSLOGDEASSOCIATIONGREPSTRING",
  "INV_TOKEN", "';'", "'{'", "'}'", "'/'", "$accept", "grammar", "topdef",
  "mip6entity", "ifacedef", "ifacesub", "ifaceopts", "ifaceopt", "linksub",
  "linkdefs", "linkdef", "ipsecpolicyset", "ipsechaaddrdef",
  "ipsecmnaddrdefs", "ipsecmnaddrdef", "ipsecmnaddr", "ipsecpolicydefs",
  "ipsecpolicydef", "ipsectype", "ipsectypeval", "ipsecprotos",
  "ipsecproto", "ipsecreqid", "xfrmaction", "mnropolicy", "mnropolicyaddr",
  "dorouteopt", "movemodule", "policymodule", "bindaclpolval",
  "bindaclpolicy", "prefixlen", "proxymiplmadef", "proxymiplmasub",
  "proxymiplmaopts", "proxymiplmaopt", "proxymipmagdef", "proxymipmagsub",
  "proxymipmagopts", "proxymipmagopt", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
      59,   123,   125,    47
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   114,   115,   115,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   117,   117,   117,
     117,   117,   118,   119,   119,   120,   120,   121,   121,   122,
     123,   123,   124,   124,   124,   124,   125,   126,   127,   127,
     128,   129,   130,   130,   131,   132,   133,   133,   133,   133,
     133,   133,   133,   133,   134,   134,   134,   134,   135,   135,
     135,   136,   136,   136,   137,   137,   138,   139,   139,   140,
     141,   141,   142,   143,   143,   144,   145,   146,   147,   147,
     148,   148,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   150,   151,   151,
     152,   152,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     3,     3,     3,     3,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     3,     3,     3,     3,     3,     3,     3,     4,
       3,     3,     3,     3,     3,     2,     2,     1,     1,     1,
       1,     1,     2,     3,     1,     1,     2,     3,     3,     4,
       1,     2,     3,     5,     3,     3,     3,     3,     1,     2,
       3,     3,     1,     2,     5,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     1,     2,     3,     1,     1,
       1,     0,     1,     2,     0,     1,     2,     0,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     2,     3,     1,
       1,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     3,     1,
       1,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,    37,    38,    39,    40,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,    91,    90,     0,    92,     0,    93,    94,     0,
       0,     0,     0,     0,     8,     0,     0,     0,     0,     0,
       0,    35,     0,    36,     1,     3,     4,     5,     6,    14,
      15,    19,    20,    16,    17,    18,    12,    13,     0,     7,
       9,    10,    11,     0,     0,     0,    22,    23,    24,    25,
      26,    95,    27,    28,    44,     0,    42,    30,    31,    32,
      33,    34,    99,     0,    97,   129,     0,   127,     0,     0,
       0,    87,     0,    50,     0,    29,     0,     0,    58,     0,
       0,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   100,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   130,     0,     0,     0,    88,     0,     0,
      49,    51,    57,     0,     0,     0,    59,    56,    62,    74,
       0,     0,    43,    46,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    98,
     101,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   128,   131,     0,    52,    54,    55,    89,
      86,     0,    60,    66,    67,    68,    69,    70,    71,    72,
      73,    65,    63,    78,    79,    80,    81,    75,    47,    48,
     106,   107,   108,   109,   110,   111,   112,   102,   103,   104,
     105,   121,   122,   123,   124,   125,   126,   113,   114,   117,
     118,   119,   120,   115,   116,   133,   134,   135,   136,   137,
     132,   139,   140,   141,   142,   138,   143,   146,   147,   148,
     149,   144,   145,   150,   151,   152,   153,    96,     0,    61,
      82,    84,    76,    53,    83,    85,     0,    77,    64
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    34,    35,    41,    74,   116,   141,   142,    54,   132,
     133,   104,   105,   137,   138,   204,   207,   208,   209,   281,
     286,   287,   341,   346,   198,   199,   270,    64,    66,    69,
      71,   338,    81,   124,   168,   169,    83,   127,   192,   193
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -286
static const yytype_int16 yypact[] =
{
      78,    -4,    -1,     9,    12,    38,    41,    44,    55,    58,
      59,    60,    66,    75,    73,    74,    76,    80,   -30,    81,
      87,     4,    82,     2,    90,    98,   113,   114,   112,   119,
     116,   122,   118,   126,     3,  -286,  -286,  -286,  -286,  -286,
    -286,    20,    21,    22,    23,    24,    27,    30,    31,    32,
      46,    47,    50,    37,  -286,    51,    56,    57,    64,   159,
      83,    84,  -286,  -286,    85,  -286,    86,  -286,  -286,    89,
       2,    91,    94,   -70,  -286,    95,    96,    97,    99,   123,
     -56,  -286,   -52,  -286,  -286,  -286,  -286,  -286,  -286,  -286,
    -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,    -5,  -286,
    -286,  -286,  -286,   180,    77,   175,  -286,  -286,  -286,  -286,
    -286,  -286,  -286,  -286,  -286,    13,  -286,  -286,  -286,  -286,
    -286,  -286,  -286,   197,  -286,  -286,   158,  -286,   193,   196,
     202,   220,   -15,  -286,   124,  -286,   221,     7,  -286,    -4,
     219,   -59,  -286,   226,   230,   233,   234,   231,   235,   236,
     240,   237,   243,   245,   246,   247,   248,   270,   271,   272,
     273,   274,   242,   277,   278,   285,   287,   296,    79,  -286,
     297,   298,   275,   300,   301,   302,   304,   305,   307,   308,
     309,   310,   311,   312,   313,   314,   306,   316,   320,   321,
     322,   323,   115,  -286,   203,   205,   207,  -286,   217,   324,
    -286,  -286,  -286,   215,   222,    69,  -286,   299,  -286,     0,
     223,   224,  -286,  -286,   225,   227,   228,   229,   232,   238,
     239,   241,   244,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,  -286,
    -286,   265,   266,   267,   268,   269,   276,   279,   280,   281,
     282,   283,   284,   286,   288,   289,   290,   291,   292,   293,
     294,   295,   303,  -286,  -286,   329,  -286,  -286,  -286,  -286,
    -286,   329,  -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,
    -286,  -286,  -286,  -286,  -286,  -286,   333,     0,  -286,  -286,
    -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,
    -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,
    -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,
    -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,
    -286,  -286,  -286,  -286,  -286,  -286,  -286,  -286,   315,  -286,
     334,   337,     0,  -286,  -286,  -286,   317,  -286,  -286
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -286,  -286,   318,   190,  -286,  -286,  -286,   204,  -286,  -286,
     212,  -286,  -286,  -286,   209,  -286,  -286,   140,  -286,  -286,
    -286,  -285,  -286,  -286,  -286,  -286,  -286,  -286,  -286,   325,
    -286,   109,  -286,  -286,  -286,   182,  -286,  -286,  -286,   161
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
     128,   129,   342,    84,   139,   140,    42,    62,    67,    68,
     128,   129,    43,     1,     2,     3,     4,     5,    44,   130,
       6,     7,   136,     8,     9,    10,    11,    12,    13,   130,
      14,    15,    16,    17,    18,   283,   284,   285,   131,   205,
     114,   115,    19,    20,    45,    36,    37,    38,   131,    46,
      21,    22,    47,   212,   122,   123,    63,   347,   125,   126,
      23,    24,    48,    25,    26,    49,    50,    51,    27,    28,
      29,    30,    31,    52,    39,    40,   139,   140,    53,    55,
      56,    59,    57,    32,    33,    65,    58,    60,     1,     2,
       3,     4,     5,    61,    70,     6,     7,   200,     8,     9,
      10,    11,    12,    13,    72,    14,    15,    16,    17,    18,
     273,   274,   275,   276,   277,   278,    73,    19,    20,    76,
      75,    80,    78,   279,   280,    21,    22,    77,    79,    82,
      86,    87,    88,    89,    90,    23,    24,    91,    25,    26,
      92,    93,    94,    27,    28,    29,    30,    31,    98,   143,
     144,   145,   146,   147,   148,   149,    95,    96,    32,    33,
      97,    99,   150,   151,   152,   153,   100,   101,   154,   155,
     156,   157,   158,   159,   102,   103,   160,   161,   162,   163,
     164,   165,   166,   167,   134,   170,   171,   172,   173,   135,
     136,   239,   174,   106,   107,   108,   109,   194,   175,   110,
     195,   112,   176,   177,   113,   117,   118,   119,   196,   120,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   197,   203,   211,   263,   170,   171,
     172,   173,   214,   121,   202,   174,   215,   216,   218,   217,
     222,   175,   219,   220,   221,   176,   177,   223,   224,   233,
     225,   226,   227,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   143,   144,   145,
     146,   147,   148,   149,   228,   229,   230,   231,   232,   243,
     150,   151,   152,   153,   234,   235,   154,   155,   156,   157,
     158,   159,   236,   237,   160,   161,   162,   163,   164,   165,
     166,   167,   238,   241,   242,   244,   246,   245,   247,   248,
     249,   250,   257,   251,   252,   266,   265,   267,   253,   254,
     255,   256,   258,   259,   260,   261,   262,   268,   271,   210,
     269,   205,   272,   288,   289,   290,   337,   291,   292,   293,
     340,   344,   294,   345,   201,   213,   206,   282,   295,   296,
     240,   297,    85,   264,   298,     0,     0,     0,     0,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     339,     0,     0,     0,     0,     0,   320,     0,     0,   321,
     322,   323,   324,   325,   326,   111,   327,     0,   328,   329,
     330,   331,   332,   333,   334,   335,     0,     0,     0,     0,
       0,     0,     0,   336,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   343,     0,   348
};

static const yytype_int16 yycheck[] =
{
      15,    16,   287,     0,    63,    64,     7,     3,     6,     7,
      15,    16,     3,    10,    11,    12,    13,    14,     6,    34,
      17,    18,    15,    20,    21,    22,    23,    24,    25,    34,
      27,    28,    29,    30,    31,    35,    36,    37,    53,    32,
     110,   111,    39,    40,     6,    49,    50,    51,    53,     8,
      47,    48,     8,   112,   110,   111,    52,   342,   110,   111,
      57,    58,     7,    60,    61,     7,     7,     7,    65,    66,
      67,    68,    69,     7,    78,    79,    63,    64,     3,     6,
       6,   111,     6,    80,    81,     3,     6,     6,    10,    11,
      12,    13,    14,     6,     4,    17,    18,   112,    20,    21,
      22,    23,    24,    25,     6,    27,    28,    29,    30,    31,
      41,    42,    43,    44,    45,    46,     3,    39,    40,     7,
       6,     3,     6,    54,    55,    47,    48,     8,     6,     3,
     110,   110,   110,   110,   110,    57,    58,   110,    60,    61,
     110,   110,   110,    65,    66,    67,    68,    69,   111,    70,
      71,    72,    73,    74,    75,    76,   110,   110,    80,    81,
     110,   110,    83,    84,    85,    86,   110,   110,    89,    90,
      91,    92,    93,    94,   110,    16,    97,    98,    99,   100,
     101,   102,   103,   104,     4,    70,    71,    72,    73,   112,
      15,   112,    77,   110,   110,   110,   110,     4,    83,   110,
       4,   110,    87,    88,   110,   110,   110,   110,     6,   110,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     4,     4,     7,   112,    70,    71,
      72,    73,     6,   110,   110,    77,     6,     4,     7,     5,
       3,    83,     7,     7,     4,    87,    88,     4,     3,     7,
       4,     4,     4,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,    70,    71,    72,
      73,    74,    75,    76,     4,     4,     4,     4,     4,     4,
      83,    84,    85,    86,     7,     7,    89,    90,    91,    92,
      93,    94,     7,     6,    97,    98,    99,   100,   101,   102,
     103,   104,     6,     6,     6,     5,     4,     6,     4,     4,
       3,     3,     6,     4,     4,   110,   113,   110,     7,     7,
       7,     7,     6,     3,     3,     3,     3,   110,   113,   139,
       6,    32,   110,   110,   110,   110,     7,   110,   110,   110,
       7,     7,   110,     6,   132,   141,   137,   207,   110,   110,
     168,   110,    34,   192,   110,    -1,    -1,    -1,    -1,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     271,    -1,    -1,    -1,    -1,    -1,   110,    -1,    -1,   110,
     110,   110,   110,   110,   110,    70,   110,    -1,   110,   110,
     110,   110,   110,   110,   110,   110,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   110,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   110,    -1,   110
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    10,    11,    12,    13,    14,    17,    18,    20,    21,
      22,    23,    24,    25,    27,    28,    29,    30,    31,    39,
      40,    47,    48,    57,    58,    60,    61,    65,    66,    67,
      68,    69,    80,    81,   115,   116,    49,    50,    51,    78,
      79,   117,     7,     3,     6,     6,     8,     8,     7,     7,
       7,     7,     7,     3,   122,     6,     6,     6,     6,   111,
       6,     6,     3,    52,   141,     3,   142,     6,     7,   143,
       4,   144,     6,     3,   118,     6,     7,     8,     6,     6,
       3,   146,     3,   150,     0,   116,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   111,   110,
     110,   110,   110,    16,   125,   126,   110,   110,   110,   110,
     110,   143,   110,   110,   110,   111,   119,   110,   110,   110,
     110,   110,   110,   111,   147,   110,   111,   151,    15,    16,
      34,    53,   123,   124,     4,   112,    15,   127,   128,    63,
      64,   120,   121,    70,    71,    72,    73,    74,    75,    76,
      83,    84,    85,    86,    89,    90,    91,    92,    93,    94,
      97,    98,    99,   100,   101,   102,   103,   104,   148,   149,
      70,    71,    72,    73,    77,    83,    87,    88,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   152,   153,     4,     4,     6,     4,   138,   139,
     112,   124,   110,     4,   129,    32,   128,   130,   131,   132,
     117,     7,   112,   121,     6,     6,     4,     5,     7,     7,
       7,     4,     3,     4,     3,     4,     4,     4,     4,     4,
       4,     4,     4,     7,     7,     7,     7,     6,     6,   112,
     149,     6,     6,     4,     5,     6,     4,     4,     4,     3,
       3,     4,     4,     7,     7,     7,     7,     6,     6,     3,
       3,     3,     3,   112,   153,   113,   110,   110,   110,     6,
     140,   113,   110,    41,    42,    43,    44,    45,    46,    54,
      55,   133,   131,    35,    36,    37,   134,   135,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,     7,   145,   145,
       7,   136,   135,   110,     7,     6,   137,   135,   110
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:

/* Line 1455 of yacc.c  */
#line 252 "gram.y"
    {
			conf.mip6_entity = (yyvsp[(2) - (3)].num);
		}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 256 "gram.y"
    {
			conf.debug_level = (yyvsp[(2) - (3)].num);
		}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 260 "gram.y"
    {
			conf.debug_log_file = (yyvsp[(2) - (3)].string);
		}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 264 "gram.y"
    {
			conf.NonVolatileBindingCache = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 269 "gram.y"
    {
			conf.SendMobPfxSols = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 273 "gram.y"
    {
			conf.SendUnsolMobPfxAdvs = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 277 "gram.y"
    {
			conf.SendMobPfxAdvs = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 281 "gram.y"
    {
			conf.MaxMobPfxAdvInterval = (yyvsp[(2) - (3)].num);
		}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 285 "gram.y"
    {
			conf.MinMobPfxAdvInterval = (yyvsp[(2) - (3)].num);
		}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 289 "gram.y"
    {
			conf.DoRouteOptimizationCN = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 293 "gram.y"
    {
			conf.DoRouteOptimizationMN = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 297 "gram.y"
    {
			if ((yyvsp[(2) - (3)].num) > MAX_BINDING_LIFETIME) {
				uerror("max allowed binding lifetime is %d",
				       MAX_BINDING_LIFETIME);
				return -1;
			}
			conf.HaMaxBindingLife = (yyvsp[(2) - (3)].num);
		}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 306 "gram.y"
    {
			if ((yyvsp[(2) - (3)].num) > MAX_BINDING_LIFETIME) {
				uerror("max allowed binding lifetime is %d",
				       MAX_BINDING_LIFETIME);
				return -1;
			}
			conf.MnMaxHaBindingLife = (yyvsp[(2) - (3)].num);
		}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 315 "gram.y"
    {
			if ((yyvsp[(2) - (3)].num) > MAX_RR_BINDING_LIFETIME) {
				uerror("max allowed binding lifetime is %d",
				       MAX_RR_BINDING_LIFETIME);
				return -1;
			}
			conf.MnMaxCnBindingLife = (yyvsp[(2) - (3)].num);
		}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 324 "gram.y"
    {
			tssetdsec(conf.InitialBindackTimeoutFirstReg_ts, (yyvsp[(2) - (3)].dec));
		}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 328 "gram.y"
    {
			tssetdsec(conf.InitialBindackTimeoutReReg_ts, (yyvsp[(2) - (3)].dec));
		}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 333 "gram.y"
    {
			conf.UseMnHaIPsec = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 337 "gram.y"
    {
			conf.KeyMngMobCapability = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 343 "gram.y"
    {
			conf.DefaultBindingAclPolicy = (yyvsp[(2) - (3)].num);
		}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 347 "gram.y"
    {
			bae = NULL;
		}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 351 "gram.y"
    {
			conf.CnBuAck = (yyvsp[(2) - (3)].bool) ? IP6_MH_BU_ACK : 0;
		}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 356 "gram.y"
    {
			conf.MnUseAllInterfaces = (yyvsp[(2) - (3)].bool) ? POL_MN_IF_DEF_PREFERENCE : 0;
		}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 360 "gram.y"
    {
			conf.MnRouterProbes = (yyvsp[(2) - (3)].num);
		}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 364 "gram.y"
    {
			if ((yyvsp[(2) - (3)].dec) > 0)
				tssetdsec(conf.MnRouterProbeTimeout_ts, (yyvsp[(2) - (3)].dec));
		}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 369 "gram.y"
    {
			conf.MnDiscardHaParamProb = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 373 "gram.y"
    {
			conf.OptimisticHandoff = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 380 "gram.y"
    { (yyval.num) = MIP6_ENTITY_CN;	}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 381 "gram.y"
    { (yyval.num) = MIP6_ENTITY_MN; }
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 382 "gram.y"
    { (yyval.num) = MIP6_ENTITY_HA; }
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 383 "gram.y"
    { (yyval.num) = MIP6_ENTITY_LMA; }
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 384 "gram.y"
    { (yyval.num) = MIP6_ENTITY_MAG; }
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 388 "gram.y"
    {
			struct net_iface *nni;
			strncpy(ni.name, (yyvsp[(1) - (2)].string), IF_NAMESIZE - 1);
			ni.ifindex = if_nametoindex((yyvsp[(1) - (2)].string));
			free((yyvsp[(1) - (2)].string));
			if (ni.ifindex <= 0) {
				uerror("invalid interface");
				return -1;
			}
			nni = malloc(sizeof(struct net_iface));
			if (nni == NULL) {
				uerror("out of memory");
				return -1;
			}
			memcpy(nni, &ni, sizeof(struct net_iface));
			list_add_tail(&nni->list, &conf.net_ifaces);
			if (is_if_ha(nni))
				homeagent_if_init(nni->ifindex);

			memset(&ni, 0, sizeof(struct net_iface));
			ni.mip6_if_entity = MIP6_ENTITY_NO;
			ni.mn_if_preference = POL_MN_IF_DEF_PREFERENCE;
		}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 422 "gram.y"
    {
			ni.mip6_if_entity = (yyvsp[(2) - (3)].num);
		}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 426 "gram.y"
    {
			ni.mn_if_preference = (yyvsp[(2) - (3)].num);
		}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 432 "gram.y"
    {
			struct home_addr_info *nhai;
			if (IN6_IS_ADDR_UNSPECIFIED(&hai.hoa.addr)) {
				uerror("No home addresses defined"
					"for homelink %d", hai.if_home);
				return -1;
			}
			strncpy(hai.name, (yyvsp[(1) - (4)].string), IF_NAMESIZE - 1);
			hai.if_home = if_nametoindex((yyvsp[(1) - (4)].string));
			free((yyvsp[(1) - (4)].string));
			if (hai.if_home <= 0) {
				uerror("invalid interface");
				return -1;
			}
			nhai = malloc(sizeof(struct home_addr_info));
			if (nhai == NULL) {
				uerror("out of memory");
				return -1;
			}
			if (hai.plen == 64) {
				struct in6_addr lladdr;
				ipv6_addr_llocal(&hai.hoa.addr, &lladdr);
				if (!addr_do(&lladdr, 64,
					     hai.if_home, NULL, NULL))
					hai.lladdr_comp = IP6_MH_BU_LLOCAL;
			}
			if (IN6_IS_ADDR_UNSPECIFIED(&hai.home_prefix)) {
				ipv6_addr_prefix(&hai.home_prefix,
						 &hai.hoa.addr, hai.plen);
				hai.home_plen = hai.plen;
			}
			memcpy(nhai, &hai, sizeof(struct home_addr_info));
			INIT_LIST_HEAD(&nhai->ro_policies);
			INIT_LIST_HEAD(&nhai->ha_list.home_agents);
			nhai->ha_list.dhaad_id = -1;
			list_splice(&hai.ro_policies, &nhai->ro_policies);
			list_add_tail(&nhai->list, &conf.home_addrs);

			memset(&hai, 0, sizeof(struct home_addr_info));
			INIT_LIST_HEAD(&hai.ro_policies);
		}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 480 "gram.y"
    {
			memcpy(&hai.ha_addr, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 484 "gram.y"
    {
			hai.hoa.addr = (yyvsp[(2) - (5)].addr);
			hai.plen = (yyvsp[(4) - (5)].num);
		}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 489 "gram.y"
    {
		        hai.altcoa = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 496 "gram.y"
    {
			if (!list_empty(&ipsec_ps.hoa_list)) {
				struct list_head *lp, *tmp;

				/* free each hoa entry */
				list_for_each_safe(lp, tmp,
						   &ipsec_ps.hoa_list) {
					struct home_addr_info *hoa;

					list_del(lp);
					hoa = list_entry(lp,
							 struct home_addr_info,
							 list);

					free(hoa);
				}
			}
			memset(&ipsec_ps, 0, sizeof(ipsec_ps));
			INIT_LIST_HEAD(&ipsec_ps.hoa_list);
		}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 519 "gram.y"
    {
			ipsec_ps.ha = (yyvsp[(2) - (3)].addr);
		}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 532 "gram.y"
    {
			struct home_addr_info *hai;

			hai = malloc(sizeof(struct home_addr_info));
			if (hai == NULL) {
				uerror("out of memory");
				return -1;
			}
			memset(hai, 0, sizeof(struct home_addr_info));
			hai->hoa.addr = (yyvsp[(1) - (3)].addr);
			hai->plen = (yyvsp[(3) - (3)].num);
			list_add_tail(&hai->list, &ipsec_ps.hoa_list);
		}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 552 "gram.y"
    {
			struct list_head *lp;

			if (IN6_IS_ADDR_UNSPECIFIED(&ipsec_ps.ha)) {
				uerror("HomeAgentAddress missing for IPsecPolicy");
				return -1;
			}
			if (list_empty(&ipsec_ps.hoa_list)) {
				uerror("HomeAddress missing for IPsecPolicy");
				return -1;
			}

			list_for_each(lp, &ipsec_ps.hoa_list) {
				struct home_addr_info *hai;
				struct ipsec_policy_entry *e;

				hai = list_entry(lp, struct home_addr_info,
						 list);

				e = malloc(sizeof(*e));
				if (e == NULL) {
					uerror("out of memory");
					return -1;
				}
				memset(e, 0, sizeof(*e));
				e->ha_addr = ipsec_ps.ha;
				e->mn_addr = hai->hoa.addr;
				e->type = (yyvsp[(1) - (5)].num);
#ifndef XFRM_MSG_MIGRATE
				switch (e->type) {
				case IPSEC_POLICY_TYPE_TUNNELHOMETESTING:
				case IPSEC_POLICY_TYPE_TUNNELMH:
				case IPSEC_POLICY_TYPE_TUNNELPAYLOAD:
					uerror("cannot use IPsec tunnel because it is not built with MIGRATE");
					return -1;
				default:
					break;
				}
#endif
#ifndef MULTIPROTO_MIGRATE
				if ((yyvsp[(2) - (5)].num) != IPSEC_PROTO_ESP) {
					uerror("only UseESP is allowed");
					return -1;
				}
#endif
				e->ipsec_protos = (yyvsp[(2) - (5)].num);
				e->reqid_toha = (yyvsp[(3) - (5)].numpair)[0];
				e->reqid_tomn = (yyvsp[(3) - (5)].numpair)[1];
				e->action = (yyvsp[(4) - (5)].bool);

				if (ipsec_policy_entry_check(&e->ha_addr,
							     &e->mn_addr,
							     e->type)) {
					uerror("overlapping IPsec policies "
					       "found for "
					       "HA %x:%x:%x:%x:%x:%x:%x:%x "
					       "MN %x:%x:%x:%x:%x:%x:%x:%x "
					       "pair\n",
					       NIP6ADDR(&e->ha_addr),
					       NIP6ADDR(&e->mn_addr));
					return -1;
				}
				list_add_tail(&e->list, &conf.ipsec_policies);
			}
		}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 619 "gram.y"
    { (yyval.num) = (yyvsp[(2) - (2)].num); }
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 622 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_HOMEREGBINDING; }
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 623 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_MH; }
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 624 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_MOBPFXDISC; }
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 625 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_TUNNELHOMETESTING; }
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 626 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_TUNNELMH; }
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 627 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_TUNNELPAYLOAD; }
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 628 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_ICMP; }
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 629 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_ANY; }
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 633 "gram.y"
    {
			uerror("IPsecPolicy must set at least one protocol");
			return -1;
		}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 637 "gram.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 638 "gram.y"
    { (yyval.num) = (yyvsp[(1) - (2)].num) | (yyvsp[(2) - (2)].num); }
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 639 "gram.y"
    { (yyval.num) = (yyvsp[(1) - (3)].num) | (yyvsp[(2) - (3)].num) | (yyvsp[(3) - (3)].num); }
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 642 "gram.y"
    { (yyval.num) = IPSEC_PROTO_ESP; }
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 643 "gram.y"
    { (yyval.num) = IPSEC_PROTO_AH; }
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 644 "gram.y"
    { (yyval.num) = IPSEC_PROTO_IPCOMP; }
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 647 "gram.y"
    { (yyval.numpair)[0] = (yyval.numpair)[1] = 0; }
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 648 "gram.y"
    { (yyval.numpair)[0] = (yyval.numpair)[1] = (yyvsp[(1) - (1)].num); }
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 649 "gram.y"
    { (yyval.numpair)[0] = (yyvsp[(1) - (2)].num); (yyval.numpair)[1] = (yyvsp[(2) - (2)].num); }
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 652 "gram.y"
    { (yyval.bool) = XFRM_POLICY_ALLOW; }
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 653 "gram.y"
    { (yyval.bool) = (yyvsp[(1) - (1)].bool) ? XFRM_POLICY_ALLOW : XFRM_POLICY_BLOCK; }
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 657 "gram.y"
    {
			struct xfrm_ro_pol *rp;
			rp = malloc(sizeof(struct xfrm_ro_pol));
			if (rp == NULL) {
				uerror("out of memory");
				return -1;
			}
			memset(rp, 0, sizeof(struct xfrm_ro_pol));
			rp->cn_addr = (yyvsp[(1) - (2)].addr);
			rp->do_ro = (yyvsp[(2) - (2)].bool);
			list_add_tail(&rp->list, &hai.ro_policies);
		}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 671 "gram.y"
    { (yyval.addr) = in6addr_any; }
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 672 "gram.y"
    { (yyval.addr) = (yyvsp[(1) - (1)].addr); }
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 675 "gram.y"
    { (yyval.bool) = (yyvsp[(1) - (1)].bool); }
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 679 "gram.y"
    {
			conf.MoveModulePath = NULL;
		}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 683 "gram.y"
    {
			conf.MoveModulePath = NULL;
		}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 689 "gram.y"
    {
			if (pmgr_init((yyvsp[(1) - (1)].string), &conf.pmgr) < 0) {
				uerror("error loading shared object %s", (yyvsp[(1) - (1)].string));
				return -1;
			}
		}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 698 "gram.y"
    {
			if ((yyvsp[(1) - (1)].bool))
				(yyval.num) = IP6_MH_BAS_ACCEPTED;
			else
				(yyval.num) = IP6_MH_BAS_PROHIBIT;
		}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 704 "gram.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 708 "gram.y"
    {
			bae = malloc(sizeof(struct policy_bind_acl_entry));
			if (bae == NULL) {
				uerror("out of memory");
				return -1;
			}
			memset(bae, 0, sizeof(struct policy_bind_acl_entry));
			bae->hoa = (yyvsp[(1) - (2)].addr);
			bae->plen = 128;
			bae->bind_policy = (yyvsp[(2) - (2)].num);
			list_add_tail(&bae->list, &conf.bind_acl);
		}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 723 "gram.y"
    {
			if ((yyvsp[(1) - (1)].num) > 128) {
				uerror("invalid prefix length %d", (yyvsp[(1) - (1)].num));
				return -1;
			}
			(yyval.num) = (yyvsp[(1) - (1)].num);
		}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 734 "gram.y"
    {
			conf.HomeNetworkPrefix = in6addr_any;
			conf.OurAddress        = in6addr_loopback;
		}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 749 "gram.y"
    {
			memcpy(&conf.LmaAddress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 753 "gram.y"
    {
			conf.LmaPmipNetworkDevice = (yyvsp[(2) - (3)].string);
		}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 757 "gram.y"
    {
			memcpy(&conf.LmaCoreNetworkAddress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 761 "gram.y"
    {
			conf.LmaCoreNetworkDevice = (yyvsp[(2) - (3)].string);
		}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 765 "gram.y"
    {
			conf.RFC5213TimestampBasedApproachInUse = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 769 "gram.y"
    {
			conf.RFC5213MobileNodeGeneratedTimestampInUse = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 773 "gram.y"
    {
			memcpy(&conf.RFC5213FixedMAGLinkLocalAddressOnAllAccessLinks, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 777 "gram.y"
    {
			memcpy(&conf.RFC5213FixedMAGLinkLayerAddressOnAllAccessLinks, &(yyvsp[(2) - (3)].macaddr), sizeof(struct in6_addr));
		}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 781 "gram.y"
    {
			struct timespec lifetime;
			lifetime.tv_sec = (yyvsp[(2) - (3)].num)/1000;
            lifetime.tv_nsec = ((yyvsp[(2) - (3)].num) % 1000)*1000000;
			conf.RFC5213MinDelayBeforeBCEDelete = lifetime;
		}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 788 "gram.y"
    {
			struct timespec lifetime;
            lifetime.tv_sec = (yyvsp[(2) - (3)].num)/1000;
            lifetime.tv_nsec = ((yyvsp[(2) - (3)].num) % 1000)*1000000;
			conf.RFC5213MaxDelayBeforeNewBCEAssign = lifetime;		
		}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 795 "gram.y"
    {
			struct timespec lifetime;
            lifetime.tv_sec = (yyvsp[(2) - (3)].num)/1000;
            lifetime.tv_nsec = ((yyvsp[(2) - (3)].num) % 1000)*1000000;
			conf.RFC5213TimestampValidityWindow = lifetime;
		}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 802 "gram.y"
    {
			memcpy(&conf.OurAddress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 806 "gram.y"
    {
			memcpy(&conf.HomeNetworkPrefix, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 810 "gram.y"
    {
			conf.TunnelingEnabled = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 814 "gram.y"
    {
			conf.DynamicTunnelingEnabled = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 818 "gram.y"
    {
			struct timespec lifetime;
            lifetime.tv_sec = (yyvsp[(2) - (3)].num)/1000;
            lifetime.tv_nsec = ((yyvsp[(2) - (3)].num) % 1000)*1000000;
			conf.PBULifeTime = lifetime;
		}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 825 "gram.y"
    {
			struct timespec lifetime;
            lifetime.tv_sec = (yyvsp[(2) - (3)].num)/1000;
            lifetime.tv_nsec = ((yyvsp[(2) - (3)].num) % 1000)*1000000;
			conf.PBALifeTime = lifetime;
		}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 832 "gram.y"
    {
			struct timespec lifetime;
            lifetime.tv_sec = (yyvsp[(2) - (3)].num)/1000;
            lifetime.tv_nsec = ((yyvsp[(2) - (3)].num) % 1000)*1000000;
			conf.RetransmissionTimeOut = lifetime;
		}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 839 "gram.y"
    {
			conf.MaxMessageRetransmissions = (yyvsp[(2) - (3)].num);
		}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 843 "gram.y"
    {
			memcpy(&conf.Mag1AddressIngress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 847 "gram.y"
    {
			memcpy(&conf.Mag1AddressEgress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 851 "gram.y"
    {
			memcpy(&conf.Mag2AddressIngress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 855 "gram.y"
    {
			memcpy(&conf.Mag2AddressEgress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 859 "gram.y"
    {
			memcpy(&conf.Mag3AddressIngress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 863 "gram.y"
    {
			memcpy(&conf.Mag3AddressEgress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 869 "gram.y"
    {
			conf.HomeNetworkPrefix = in6addr_any;
			conf.OurAddress        = in6addr_loopback;
		}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 884 "gram.y"
    {
			memcpy(&conf.LmaAddress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 888 "gram.y"
    {
			conf.RFC5213TimestampBasedApproachInUse = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 892 "gram.y"
    {
			conf.RFC5213MobileNodeGeneratedTimestampInUse = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 896 "gram.y"
    {
			memcpy(&conf.RFC5213FixedMAGLinkLocalAddressOnAllAccessLinks, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 900 "gram.y"
    {
			memcpy(&conf.RFC5213FixedMAGLinkLayerAddressOnAllAccessLinks, &(yyvsp[(2) - (3)].macaddr), sizeof(struct in6_addr));
		}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 904 "gram.y"
    {
			conf.RFC5213EnableMAGLocalRouting = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 908 "gram.y"
    {
			memcpy(&conf.OurAddress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 912 "gram.y"
    {
			memcpy(&conf.MagAddressIngress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 916 "gram.y"
    {
			memcpy(&conf.MagAddressEgress, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 920 "gram.y"
    {
			conf.MagDeviceIngress = (yyvsp[(2) - (3)].string);		
		}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 924 "gram.y"
    {
			conf.MagDeviceEgress = (yyvsp[(2) - (3)].string);
		}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 928 "gram.y"
    {
			memcpy(&conf.HomeNetworkPrefix, &(yyvsp[(2) - (3)].addr), sizeof(struct in6_addr));
		}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 932 "gram.y"
    {
			conf.TunnelingEnabled = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 936 "gram.y"
    {
			conf.DynamicTunnelingEnabled = (yyvsp[(2) - (3)].bool);
		}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 940 "gram.y"
    {
			struct timespec lifetime;
            lifetime.tv_sec = (yyvsp[(2) - (3)].num)/1000;
            lifetime.tv_nsec = ((yyvsp[(2) - (3)].num) % 1000)*1000000;
			conf.PBULifeTime = lifetime;
		}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 947 "gram.y"
    {
			struct timespec lifetime;
            lifetime.tv_sec = (yyvsp[(2) - (3)].num)/1000;
            lifetime.tv_nsec = ((yyvsp[(2) - (3)].num) % 1000)*1000000;
			conf.PBALifeTime = lifetime;
		}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 954 "gram.y"
    {
			struct timespec lifetime;
            lifetime.tv_sec = (yyvsp[(2) - (3)].num)/1000;
            lifetime.tv_nsec = ((yyvsp[(2) - (3)].num) % 1000)*1000000;
			conf.RetransmissionTimeOut = lifetime;
		}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 961 "gram.y"
    {
			conf.MaxMessageRetransmissions = (yyvsp[(2) - (3)].num);
		}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 965 "gram.y"
    {
			conf.RadiusPassword = (yyvsp[(2) - (3)].string);
		}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 969 "gram.y"
    {
			conf.RadiusClientConfigFile = (yyvsp[(2) - (3)].string);
		}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 973 "gram.y"
    {
			conf.PcapSyslogAssociationGrepString = (yyvsp[(2) - (3)].string);
		}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 977 "gram.y"
    {
			conf.PcapSyslogDeAssociationGrepString = (yyvsp[(2) - (3)].string);
		}
    break;



/* Line 1455 of yacc.c  */
#line 3231 "gram.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 982 "gram.y"


