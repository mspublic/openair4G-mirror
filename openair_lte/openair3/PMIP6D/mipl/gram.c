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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 28 "gram.y"


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
/* Line 196 of yacc.c.  */
#line 294 "gram.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 219 of yacc.c.  */
#line 306 "gram.c"

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T) && (defined (__STDC__) || defined (__cplusplus))
# include <stddef.h> /* INFRINGES ON USER NAME SPACE */
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

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

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if defined (__STDC__) || defined (__cplusplus)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     define YYINCLUDED_STDLIB_H
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2005 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM ((YYSIZE_T) -1)
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if (! defined (malloc) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if (! defined (free) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifdef __cplusplus
}
#  endif
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
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
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  74
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   237

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  32
/* YYNRULES -- Number of rules. */
#define YYNRULES  91
/* YYNRULES -- Number of states. */
#define YYNSTATES  183

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   323

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    72,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    69,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    70,     2,    71,     2,     2,     2,     2,
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
      65,    66,    67,    68
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     8,    12,    16,    20,    23,    27,
      31,    35,    39,    43,    47,    51,    55,    59,    63,    67,
      71,    74,    78,    82,    86,    90,    94,    98,   102,   107,
     111,   115,   119,   123,   127,   129,   131,   133,   136,   140,
     142,   144,   147,   151,   155,   160,   162,   165,   169,   175,
     179,   183,   187,   191,   193,   196,   200,   204,   206,   209,
     215,   218,   220,   222,   224,   226,   228,   230,   232,   234,
     235,   237,   240,   244,   246,   248,   250,   251,   253,   256,
     257,   259,   262,   263,   265,   267,   269,   271,   273,   275,
     277,   280
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      74,     0,    -1,    75,    -1,    74,    75,    -1,     9,    76,
      69,    -1,    10,     6,    69,    -1,    25,     5,    69,    -1,
      59,    77,    -1,    26,     5,    69,    -1,    27,     5,    69,
      -1,    28,     5,    69,    -1,    21,     6,    69,    -1,    22,
       6,    69,    -1,    11,     5,    69,    -1,    12,     5,    69,
      -1,    18,     6,    69,    -1,    19,     6,    69,    -1,    20,
       6,    69,    -1,    15,     7,    69,    -1,    16,     7,    69,
      -1,    23,    81,    -1,    37,     5,    69,    -1,    38,     5,
      69,    -1,    45,   100,    69,    -1,    46,   101,    69,    -1,
      55,   102,    69,    -1,    56,   103,    69,    -1,    58,     5,
      69,    -1,    29,    70,    84,    71,    -1,    63,     5,    69,
      -1,    64,     6,    69,    -1,    65,     7,    69,    -1,    66,
       5,    69,    -1,    67,     5,    69,    -1,    47,    -1,    48,
      -1,    49,    -1,     3,    78,    -1,    70,    79,    71,    -1,
      69,    -1,    80,    -1,    79,    80,    -1,    61,    76,    69,
      -1,    62,     6,    69,    -1,     3,    70,    82,    71,    -1,
      83,    -1,    82,    83,    -1,    14,     4,    69,    -1,    13,
       4,    72,   104,    69,    -1,    32,     5,    69,    -1,    51,
      97,    69,    -1,    85,    86,    89,    -1,    14,     4,    69,
      -1,    87,    -1,    86,    87,    -1,    13,    88,    69,    -1,
       4,    72,   104,    -1,    90,    -1,    89,    90,    -1,    91,
      93,    95,    96,    69,    -1,    30,    92,    -1,    39,    -1,
      40,    -1,    41,    -1,    42,    -1,    43,    -1,    44,    -1,
      52,    -1,    53,    -1,    -1,    94,    -1,    94,    94,    -1,
      94,    94,    94,    -1,    33,    -1,    34,    -1,    35,    -1,
      -1,     6,    -1,     6,     6,    -1,    -1,     5,    -1,    98,
      99,    -1,    -1,     4,    -1,     5,    -1,    50,    -1,     3,
      -1,     3,    -1,     5,    -1,     6,    -1,     4,   102,    -1,
       6,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   185,   185,   186,   189,   193,   197,   201,   202,   206,
     210,   214,   218,   222,   226,   230,   239,   248,   257,   261,
     265,   266,   270,   274,   275,   276,   280,   284,   288,   289,
     293,   297,   302,   306,   312,   313,   314,   317,   343,   344,
     347,   348,   351,   355,   361,   405,   406,   409,   413,   418,
     422,   425,   448,   454,   455,   458,   461,   477,   478,   481,
     538,   541,   542,   543,   544,   545,   546,   547,   548,   552,
     556,   557,   558,   561,   562,   563,   566,   567,   568,   571,
     572,   575,   590,   591,   594,   597,   601,   607,   616,   623,
     626,   641
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "QSTRING", "ADDR", "BOOL", "NUMBER",
  "DECIMAL", "NUMPAIR", "MIP6ENTITY", "DEBUGLEVEL",
  "DOROUTEOPTIMIZATIONCN", "DOROUTEOPTIMIZATIONMN", "HOMEADDRESS",
  "HOMEAGENTADDRESS", "INITIALBINDACKTIMEOUTFIRSTREG",
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
  "INV_TOKEN", "';'", "'{'", "'}'", "'/'", "$accept", "grammar", "topdef",
  "mip6entity", "ifacedef", "ifacesub", "ifaceopts", "ifaceopt", "linksub",
  "linkdefs", "linkdef", "ipsecpolicyset", "ipsechaaddrdef",
  "ipsecmnaddrdefs", "ipsecmnaddrdef", "ipsecmnaddr", "ipsecpolicydefs",
  "ipsecpolicydef", "ipsectype", "ipsectypeval", "ipsecprotos",
  "ipsecproto", "ipsecreqid", "xfrmaction", "mnropolicy", "mnropolicyaddr",
  "dorouteopt", "movemodule", "policymodule", "bindaclpolval",
  "bindaclpolicy", "prefixlen", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,    59,
     123,   125,    47
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    73,    74,    74,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    76,    76,    76,    77,    78,    78,
      79,    79,    80,    80,    81,    82,    82,    83,    83,    83,
      83,    84,    85,    86,    86,    87,    88,    89,    89,    90,
      91,    92,    92,    92,    92,    92,    92,    92,    92,    93,
      93,    93,    93,    94,    94,    94,    95,    95,    95,    96,
      96,    97,    98,    98,    99,   100,   100,   101,   102,   102,
     103,   104
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     2,     3,     3,     3,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     3,     3,     3,     3,     3,     3,     3,     4,     3,
       3,     3,     3,     3,     1,     1,     1,     2,     3,     1,
       1,     2,     3,     3,     4,     1,     2,     3,     5,     3,
       3,     3,     3,     1,     2,     3,     3,     1,     2,     5,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       1,     2,     3,     1,     1,     1,     0,     1,     2,     0,
       1,     2,     0,     1,     1,     1,     1,     1,     1,     1,
       2,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     2,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
       0,     0,     0,     0,     0,     0,    86,    85,     0,    87,
       0,    88,    89,     0,     0,     0,     0,     0,     7,     0,
       0,     0,     0,     0,     1,     3,     4,     5,    13,    14,
      18,    19,    15,    16,    17,    11,    12,     0,     6,     8,
       9,    10,     0,     0,     0,    21,    22,    23,    24,    25,
      90,    26,    27,    39,     0,    37,    29,    30,    31,    32,
      33,     0,     0,     0,    82,     0,    45,     0,    28,     0,
       0,    53,     0,     0,     0,    40,     0,     0,     0,    83,
       0,     0,    44,    46,    52,     0,     0,     0,    54,    51,
      57,    69,     0,     0,    38,    41,     0,    47,    49,    50,
      84,    81,     0,    55,    61,    62,    63,    64,    65,    66,
      67,    68,    60,    58,    73,    74,    75,    76,    70,    42,
      43,    91,     0,    56,    77,    79,    71,    48,    78,    80,
       0,    72,    59
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    31,    32,    36,    68,   105,   124,   125,    48,   115,
     116,    93,    94,   120,   121,   136,   139,   140,   141,   162,
     167,   168,   175,   180,   130,   131,   151,    58,    60,    63,
      65,   172
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -137
static const short int yypact[] =
{
      59,     2,    -4,    12,    25,    26,    29,    33,    38,    41,
      46,    48,    54,    68,    71,    78,    86,    23,    89,    90,
      -2,    95,    36,    96,    94,    98,    97,   106,   109,   108,
     116,     0,  -137,  -137,  -137,  -137,    34,    58,    60,    61,
      62,    63,    64,    65,    66,    69,    70,    72,  -137,    74,
      75,    76,    77,   114,    79,    80,  -137,  -137,    81,  -137,
      82,  -137,  -137,    83,    36,    84,    85,   -62,  -137,    87,
      88,    91,    92,    93,  -137,  -137,  -137,  -137,  -137,  -137,
    -137,  -137,  -137,  -137,  -137,  -137,  -137,    -8,  -137,  -137,
    -137,  -137,   132,    99,   124,  -137,  -137,  -137,  -137,  -137,
    -137,  -137,  -137,  -137,    28,  -137,  -137,  -137,  -137,  -137,
    -137,   136,   137,   142,   151,    21,  -137,   100,  -137,   154,
       1,  -137,     2,   153,   -58,  -137,   101,   102,   103,  -137,
     105,   158,  -137,  -137,  -137,   104,   110,    67,  -137,   134,
    -137,    27,   111,   112,  -137,  -137,   159,  -137,  -137,  -137,
    -137,  -137,   159,  -137,  -137,  -137,  -137,  -137,  -137,  -137,
    -137,  -137,  -137,  -137,  -137,  -137,  -137,   160,    27,  -137,
    -137,  -137,   113,  -137,   161,   163,    27,  -137,  -137,  -137,
     115,  -137,  -137
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -137,  -137,   144,    55,  -137,  -137,  -137,    73,  -137,  -137,
     107,  -137,  -137,  -137,   117,  -137,  -137,    39,  -137,  -137,
    -137,  -136,  -137,  -137,  -137,  -137,  -137,  -137,  -137,   119,
    -137,    35
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      74,    56,    37,   122,   123,   111,   112,   103,   104,     1,
       2,     3,     4,   144,   119,     5,     6,    38,     7,     8,
       9,    10,    11,    12,   113,    13,    14,    15,    16,    17,
      39,   137,   176,    40,   111,   112,    41,    18,    19,    42,
     181,    61,    62,   114,    43,    20,    21,    44,    57,    33,
      34,    35,    45,   113,    46,    22,    23,    47,    24,    25,
     164,   165,   166,    26,    27,    28,    29,    30,     1,     2,
       3,     4,   114,    49,     5,     6,    50,     7,     8,     9,
      10,    11,    12,    51,    13,    14,    15,    16,    17,   122,
     123,    52,   132,    53,    54,    55,    18,    19,    59,    66,
      64,    67,    69,    76,    20,    21,   154,   155,   156,   157,
     158,   159,    70,    72,    22,    23,    71,    24,    25,   160,
     161,    73,    26,    27,    28,    29,    30,    77,    92,    78,
      79,    80,    81,    82,    83,    84,   117,   119,    85,    86,
     126,   127,    87,    88,    89,    90,    91,   128,    95,    96,
      97,    98,    99,   101,   102,   129,   106,   107,   135,   143,
     108,   109,   110,   150,   137,   171,   174,   178,   179,   134,
     118,   147,   148,   146,   149,    75,   152,   142,   163,   153,
     169,   170,   177,   100,   182,     0,     0,   173,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   145,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   133,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   138
};

static const short int yycheck[] =
{
       0,     3,     6,    61,    62,    13,    14,    69,    70,     9,
      10,    11,    12,    71,    13,    15,    16,     5,    18,    19,
      20,    21,    22,    23,    32,    25,    26,    27,    28,    29,
       5,    30,   168,     7,    13,    14,     7,    37,    38,     6,
     176,     5,     6,    51,     6,    45,    46,     6,    50,    47,
      48,    49,     6,    32,     6,    55,    56,     3,    58,    59,
      33,    34,    35,    63,    64,    65,    66,    67,     9,    10,
      11,    12,    51,     5,    15,    16,     5,    18,    19,    20,
      21,    22,    23,     5,    25,    26,    27,    28,    29,    61,
      62,     5,    71,    70,     5,     5,    37,    38,     3,     5,
       4,     3,     5,    69,    45,    46,    39,    40,    41,    42,
      43,    44,     6,     5,    55,    56,     7,    58,    59,    52,
      53,     5,    63,    64,    65,    66,    67,    69,    14,    69,
      69,    69,    69,    69,    69,    69,     4,    13,    69,    69,
       4,     4,    70,    69,    69,    69,    69,     5,    69,    69,
      69,    69,    69,    69,    69,     4,    69,    69,     4,     6,
      69,    69,    69,     5,    30,     6,     6,     6,     5,    69,
      71,    69,    69,    72,    69,    31,    72,   122,   139,    69,
      69,    69,    69,    64,    69,    -1,    -1,   152,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     9,    10,    11,    12,    15,    16,    18,    19,    20,
      21,    22,    23,    25,    26,    27,    28,    29,    37,    38,
      45,    46,    55,    56,    58,    59,    63,    64,    65,    66,
      67,    74,    75,    47,    48,    49,    76,     6,     5,     5,
       7,     7,     6,     6,     6,     6,     6,     3,    81,     5,
       5,     5,     5,    70,     5,     5,     3,    50,   100,     3,
     101,     5,     6,   102,     4,   103,     5,     3,    77,     5,
       6,     7,     5,     5,     0,    75,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    70,    69,    69,
      69,    69,    14,    84,    85,    69,    69,    69,    69,    69,
     102,    69,    69,    69,    70,    78,    69,    69,    69,    69,
      69,    13,    14,    32,    51,    82,    83,     4,    71,    13,
      86,    87,    61,    62,    79,    80,     4,     4,     5,     4,
      97,    98,    71,    83,    69,     4,    88,    30,    87,    89,
      90,    91,    76,     6,    71,    80,    72,    69,    69,    69,
       5,    99,    72,    69,    39,    40,    41,    42,    43,    44,
      52,    53,    92,    90,    33,    34,    35,    93,    94,    69,
      69,     6,   104,   104,     6,    95,    94,    69,     6,     5,
      96,    94,    69
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
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
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
    while (0)
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
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr,					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname[yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

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
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
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
      size_t yyn = 0;
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

#endif /* YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
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
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

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
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()
    ;
#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

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
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


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
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
#line 190 "gram.y"
    {
			conf.mip6_entity = (yyvsp[-1].num);
		}
    break;

  case 5:
#line 194 "gram.y"
    {
			conf.debug_level = (yyvsp[-1].num);
		}
    break;

  case 6:
#line 198 "gram.y"
    {
			conf.NonVolatileBindingCache = (yyvsp[-1].bool);
		}
    break;

  case 8:
#line 203 "gram.y"
    {
			conf.SendMobPfxSols = (yyvsp[-1].bool);
		}
    break;

  case 9:
#line 207 "gram.y"
    {
			conf.SendUnsolMobPfxAdvs = (yyvsp[-1].bool);
		}
    break;

  case 10:
#line 211 "gram.y"
    {
			conf.SendMobPfxAdvs = (yyvsp[-1].bool);
		}
    break;

  case 11:
#line 215 "gram.y"
    {
			conf.MaxMobPfxAdvInterval = (yyvsp[-1].num);
		}
    break;

  case 12:
#line 219 "gram.y"
    {
			conf.MinMobPfxAdvInterval = (yyvsp[-1].num);
		}
    break;

  case 13:
#line 223 "gram.y"
    {
			conf.DoRouteOptimizationCN = (yyvsp[-1].bool);
		}
    break;

  case 14:
#line 227 "gram.y"
    {
			conf.DoRouteOptimizationMN = (yyvsp[-1].bool);
		}
    break;

  case 15:
#line 231 "gram.y"
    {
			if ((yyvsp[-1].num) > MAX_BINDING_LIFETIME) {
				uerror("max allowed binding lifetime is %d", 
				       MAX_BINDING_LIFETIME);
				return -1;
			}
			conf.HaMaxBindingLife = (yyvsp[-1].num);
		}
    break;

  case 16:
#line 240 "gram.y"
    {
			if ((yyvsp[-1].num) > MAX_BINDING_LIFETIME) {
				uerror("max allowed binding lifetime is %d", 
				       MAX_BINDING_LIFETIME);
				return -1;
			}
			conf.MnMaxHaBindingLife = (yyvsp[-1].num);
		}
    break;

  case 17:
#line 249 "gram.y"
    {
			if ((yyvsp[-1].num) > MAX_RR_BINDING_LIFETIME) {
				uerror("max allowed binding lifetime is %d", 
				       MAX_RR_BINDING_LIFETIME);
				return -1;
			}
			conf.MnMaxCnBindingLife = (yyvsp[-1].num);
		}
    break;

  case 18:
#line 258 "gram.y"
    {
			tssetdsec(conf.InitialBindackTimeoutFirstReg_ts, (yyvsp[-1].dec));
		}
    break;

  case 19:
#line 262 "gram.y"
    {
			tssetdsec(conf.InitialBindackTimeoutReReg_ts, (yyvsp[-1].dec));
		}
    break;

  case 21:
#line 267 "gram.y"
    {
			conf.UseMnHaIPsec = (yyvsp[-1].bool);
		}
    break;

  case 22:
#line 271 "gram.y"
    {
			conf.KeyMngMobCapability = (yyvsp[-1].bool);
		}
    break;

  case 25:
#line 277 "gram.y"
    {
			conf.DefaultBindingAclPolicy = (yyvsp[-1].num);
		}
    break;

  case 26:
#line 281 "gram.y"
    {
			bae = NULL;
		}
    break;

  case 27:
#line 285 "gram.y"
    {
			conf.CnBuAck = (yyvsp[-1].bool) ? IP6_MH_BU_ACK : 0;
		}
    break;

  case 29:
#line 290 "gram.y"
    {
			conf.MnUseAllInterfaces = (yyvsp[-1].bool) ? POL_MN_IF_DEF_PREFERENCE : 0;
		}
    break;

  case 30:
#line 294 "gram.y"
    {
			conf.MnRouterProbes = (yyvsp[-1].num);
		}
    break;

  case 31:
#line 298 "gram.y"
    {
			if ((yyvsp[-1].dec) > 0)
				tssetdsec(conf.MnRouterProbeTimeout_ts, (yyvsp[-1].dec));
		}
    break;

  case 32:
#line 303 "gram.y"
    {
			conf.MnDiscardHaParamProb = (yyvsp[-1].bool);
		}
    break;

  case 33:
#line 307 "gram.y"
    {
			conf.OptimisticHandoff = (yyvsp[-1].bool);
		}
    break;

  case 34:
#line 312 "gram.y"
    { (yyval.num) = MIP6_ENTITY_CN;	}
    break;

  case 35:
#line 313 "gram.y"
    { (yyval.num) = MIP6_ENTITY_MN; }
    break;

  case 36:
#line 314 "gram.y"
    { (yyval.num) = MIP6_ENTITY_HA; }
    break;

  case 37:
#line 318 "gram.y"
    {
			struct net_iface *nni;
			strncpy(ni.name, (yyvsp[-1].string), IF_NAMESIZE - 1);
			ni.ifindex = if_nametoindex((yyvsp[-1].string));
			free((yyvsp[-1].string));
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

  case 42:
#line 352 "gram.y"
    {
			ni.mip6_if_entity = (yyvsp[-1].num);
		}
    break;

  case 43:
#line 356 "gram.y"
    {
			ni.mn_if_preference = (yyvsp[-1].num);
		}
    break;

  case 44:
#line 362 "gram.y"
    {
			struct home_addr_info *nhai;
			if (IN6_IS_ADDR_UNSPECIFIED(&hai.hoa.addr)) {
				uerror("No home addresses defined"
					"for homelink %d", hai.if_home);
				return -1;
			}
			strncpy(hai.name, (yyvsp[-3].string), IF_NAMESIZE - 1);
			hai.if_home = if_nametoindex((yyvsp[-3].string));
			free((yyvsp[-3].string));
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

  case 47:
#line 410 "gram.y"
    {
			memcpy(&hai.ha_addr, &(yyvsp[-1].addr), sizeof(struct in6_addr));
		}
    break;

  case 48:
#line 414 "gram.y"
    {
			hai.hoa.addr = (yyvsp[-3].addr);
			hai.plen = (yyvsp[-1].num);
		}
    break;

  case 49:
#line 419 "gram.y"
    {
		        hai.altcoa = (yyvsp[-1].bool);
		}
    break;

  case 51:
#line 426 "gram.y"
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

  case 52:
#line 449 "gram.y"
    {
			ipsec_ps.ha = (yyvsp[-1].addr);
		}
    break;

  case 56:
#line 462 "gram.y"
    {
			struct home_addr_info *hai;

			hai = malloc(sizeof(struct home_addr_info));
			if (hai == NULL) {
				uerror("out of memory");
				return -1;
			}
			memset(hai, 0, sizeof(struct home_addr_info)); 
			hai->hoa.addr = (yyvsp[-2].addr);
			hai->plen = (yyvsp[0].num);
			list_add_tail(&hai->list, &ipsec_ps.hoa_list);
		}
    break;

  case 59:
#line 482 "gram.y"
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
				e->type = (yyvsp[-4].num);
#ifndef MULTIPROTO_MIGRATE
				if ((yyvsp[-3].num) != IPSEC_PROTO_ESP) {
					uerror("only UseESP is allowed");
					return -1;
				}
#endif
				e->ipsec_protos = (yyvsp[-3].num);
				e->reqid_toha = (yyvsp[-2].numpair)[0];
				e->reqid_tomn = (yyvsp[-2].numpair)[1];
				e->action = (yyvsp[-1].bool);

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

  case 60:
#line 538 "gram.y"
    { (yyval.num) = (yyvsp[0].num); }
    break;

  case 61:
#line 541 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_HOMEREGBINDING; }
    break;

  case 62:
#line 542 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_MH; }
    break;

  case 63:
#line 543 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_MOBPFXDISC; }
    break;

  case 64:
#line 544 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_TUNNELHOMETESTING; }
    break;

  case 65:
#line 545 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_TUNNELMH; }
    break;

  case 66:
#line 546 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_TUNNELPAYLOAD; }
    break;

  case 67:
#line 547 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_ICMP; }
    break;

  case 68:
#line 548 "gram.y"
    { (yyval.num) = IPSEC_POLICY_TYPE_ANY; }
    break;

  case 69:
#line 552 "gram.y"
    {
			uerror("IPsecPolicy must set at least one protocol");
			return -1;
		}
    break;

  case 70:
#line 556 "gram.y"
    { (yyval.num) = (yyvsp[0].num); }
    break;

  case 71:
#line 557 "gram.y"
    { (yyval.num) = (yyvsp[-1].num) | (yyvsp[0].num); }
    break;

  case 72:
#line 558 "gram.y"
    { (yyval.num) = (yyvsp[-2].num) | (yyvsp[-1].num) | (yyvsp[0].num); }
    break;

  case 73:
#line 561 "gram.y"
    { (yyval.num) = IPSEC_PROTO_ESP; }
    break;

  case 74:
#line 562 "gram.y"
    { (yyval.num) = IPSEC_PROTO_AH; }
    break;

  case 75:
#line 563 "gram.y"
    { (yyval.num) = IPSEC_PROTO_IPCOMP; }
    break;

  case 76:
#line 566 "gram.y"
    { (yyval.numpair)[0] = (yyval.numpair)[1] = 0; }
    break;

  case 77:
#line 567 "gram.y"
    { (yyval.numpair)[0] = (yyval.numpair)[1] = (yyvsp[0].num); }
    break;

  case 78:
#line 568 "gram.y"
    { (yyval.numpair)[0] = (yyvsp[-1].num); (yyval.numpair)[1] = (yyvsp[0].num); }
    break;

  case 79:
#line 571 "gram.y"
    { (yyval.bool) = XFRM_POLICY_ALLOW; }
    break;

  case 80:
#line 572 "gram.y"
    { (yyval.bool) = (yyvsp[0].bool) ? XFRM_POLICY_ALLOW : XFRM_POLICY_BLOCK; }
    break;

  case 81:
#line 576 "gram.y"
    { 
			struct xfrm_ro_pol *rp;
			rp = malloc(sizeof(struct xfrm_ro_pol));
			if (rp == NULL) {
				uerror("out of memory");
				return -1;
			}
			memset(rp, 0, sizeof(struct xfrm_ro_pol)); 
			rp->cn_addr = (yyvsp[-1].addr);
			rp->do_ro = (yyvsp[0].bool);
			list_add_tail(&rp->list, &hai.ro_policies);
		}
    break;

  case 82:
#line 590 "gram.y"
    { (yyval.addr) = in6addr_any; }
    break;

  case 83:
#line 591 "gram.y"
    { (yyval.addr) = (yyvsp[0].addr); }
    break;

  case 84:
#line 594 "gram.y"
    { (yyval.bool) = (yyvsp[0].bool); }
    break;

  case 85:
#line 598 "gram.y"
    {
			conf.MoveModulePath = NULL;
		}
    break;

  case 86:
#line 602 "gram.y"
    {
			conf.MoveModulePath = NULL;
		}
    break;

  case 87:
#line 608 "gram.y"
    {
			if (pmgr_init((yyvsp[0].string), &conf.pmgr) < 0) {
				uerror("error loading shared object %s", (yyvsp[0].string));
				return -1;
			}
		}
    break;

  case 88:
#line 617 "gram.y"
    { 
			if ((yyvsp[0].bool))
				(yyval.num) = IP6_MH_BAS_ACCEPTED;
			else
				(yyval.num) = IP6_MH_BAS_PROHIBIT;
		}
    break;

  case 89:
#line 623 "gram.y"
    { (yyval.num) = (yyvsp[0].num); }
    break;

  case 90:
#line 627 "gram.y"
    {
			bae = malloc(sizeof(struct policy_bind_acl_entry));
			if (bae == NULL) {
				uerror("out of memory");
				return -1;
			}
			memset(bae, 0, sizeof(struct policy_bind_acl_entry)); 
			bae->hoa = (yyvsp[-1].addr);
			bae->plen = 128;
			bae->bind_policy = (yyvsp[0].num);
			list_add_tail(&bae->list, &conf.bind_acl);
		}
    break;

  case 91:
#line 642 "gram.y"
    {
			if ((yyvsp[0].num) > 128) {
				uerror("invalid prefix length %d", (yyvsp[0].num));
				return -1;
			}
			(yyval.num) = (yyvsp[0].num);
		}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 2111 "gram.c"

  yyvsp -= yylen;
  yyssp -= yylen;


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
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  int yytype = YYTRANSLATE (yychar);
	  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
	  YYSIZE_T yysize = yysize0;
	  YYSIZE_T yysize1;
	  int yysize_overflow = 0;
	  char *yymsg = 0;
#	  define YYERROR_VERBOSE_ARGS_MAXIMUM 5
	  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
	  int yyx;

#if 0
	  /* This is so xgettext sees the translatable formats that are
	     constructed on the fly.  */
	  YY_("syntax error, unexpected %s");
	  YY_("syntax error, unexpected %s, expecting %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
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
	  int yychecklim = YYLAST - yyn;
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
		yysize_overflow |= yysize1 < yysize;
		yysize = yysize1;
		yyfmt = yystpcpy (yyfmt, yyprefix);
		yyprefix = yyor;
	      }

	  yyf = YY_(yyformat);
	  yysize1 = yysize + yystrlen (yyf);
	  yysize_overflow |= yysize1 < yysize;
	  yysize = yysize1;

	  if (!yysize_overflow && yysize <= YYSTACK_ALLOC_MAXIMUM)
	    yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg)
	    {
	      /* Avoid sprintf, as that infringes on the user's name space.
		 Don't have undefined behavior even if the translation
		 produced a string with the wrong number of "%s"s.  */
	      char *yyp = yymsg;
	      int yyi = 0;
	      while ((*yyp = *yyf))
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
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    {
	      yyerror (YY_("syntax error"));
	      goto yyexhaustedlab;
	    }
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (YY_("syntax error"));
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (0)
     goto yyerrorlab;

yyvsp -= yylen;
  yyssp -= yylen;
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


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK;
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 651 "gram.y"


