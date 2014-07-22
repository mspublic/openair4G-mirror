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

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

*******************************************************************************/
/*

Source      commonDef.h

Version     0.1

Date        2012/02/27

Product     NAS stack

Subsystem   include

Author      Frederic Maurel

Description Contains global common definitions

*****************************************************************************/
#ifndef __COMMONDEF_H__
#define __COMMONDEF_H__

#include <stdint.h>
#include <stddef.h>

/****************************************************************************/
/*********************  G L O B A L    C O N S T A N T S  *******************/
/****************************************************************************/

#define RETURNok        (0)
#define RETURNerror     (-1)

#ifndef FALSE
#define FALSE           (0)
#endif
#ifndef TRUE
#define TRUE            (1)
#endif

/*
 * Name of the environment variable which defines the default directory
 * where the NAS application is executed and where are located files
 * where non-volatile data are stored
 */
#define DEFAULT_NAS_PATH    "PWD"

/****************************************************************************/
/************************  G L O B A L    T Y P E S  ************************/
/****************************************************************************/

/*
-----------------------------------------------------------------------------
            Standard data type definitions
-----------------------------------------------------------------------------
*/
typedef int8_t      SByte_t;    /* 8 bit  signed integer     */
typedef int8_t      Int8_t;     /* 8 bit  signed integer     */
typedef int16_t     Int16_t;    /* 16 bit signed integer    */
typedef int32_t     Int32_t;    /* 32 bit signed integer    */
typedef int64_t     Int64_t;    /* 64 bit signed integer    */


typedef uint8_t     Byte_t;     /* 8 bit unsigned integer   */
typedef uint8_t     UInt8_t;    /* 8 bit unsigned integer   */
typedef uint16_t    UInt16_t;   /* 16 bit unsigned integer  */
typedef uint32_t    UInt32_t;   /* 32 bit unsigned integer  */
typedef uint64_t    UInt64_t;   /* 64 bit unsigned integer  */

/*
-----------------------------------------------------------------------------
            Common NAS data type definitions
-----------------------------------------------------------------------------
*/

typedef UInt8_t     Stat_t;     /* Registration status  */
typedef UInt16_t    lac_t;      /* Location Area Code   */
typedef UInt8_t     rac_t;      /* Routing Area Code    */
typedef UInt16_t    tac_t;      /* Tracking Area Code   */
typedef UInt32_t    ci_t;       /* Cell Identifier  */
typedef UInt8_t     AcT_t;      /* Access Technology    */

/*
 * International Mobile Subscriber Identity
 */
typedef struct {
    Byte_t length;
    union {
        struct {
            Byte_t digit2:4;
            Byte_t digit1:4;
            Byte_t digit4:4;
            Byte_t digit3:4;
            Byte_t digit6:4;
            Byte_t digit5:4;
            Byte_t digit8:4;
            Byte_t digit7:4;
            Byte_t digit10:4;
            Byte_t digit9:4;
            Byte_t digit12:4;
            Byte_t digit11:4;
            Byte_t digit14:4;
            Byte_t digit13:4;
#define EVEN_PARITY 0
#define ODD_PARITY  1
            Byte_t parity:4;
            Byte_t digit15:4;
        } num;
#define IMSI_SIZE   8
        Byte_t value[IMSI_SIZE];
    } u;
} imsi_t;

/*
 * Mobile subscriber dialing number
 */
typedef struct {
    Byte_t ext:1;
    /* Type Of Number           */
#define MSISDN_TON_UNKNOWKN     0b000
#define MSISDN_TON_INTERNATIONAL    0b001
#define MSISDN_TON_NATIONAL     0b010
#define MSISDN_TON_NETWORK      0b011
#define MSISDN_TON_SUBCRIBER        0b100
#define MSISDN_TON_ABBREVIATED      0b110
#define MSISDN_TON_RESERVED     0b111
    Byte_t ton:3;
    /* Numbering Plan Identification    */
#define MSISDN_NPI_UNKNOWN      0b0000
#define MSISDN_NPI_ISDN_TELEPHONY   0b0001
#define MSISDN_NPI_GENERIC      0b0010
#define MSISDN_NPI_DATA         0b0011
#define MSISDN_NPI_TELEX        0b0100
#define MSISDN_NPI_MARITIME_MOBILE  0b0101
#define MSISDN_NPI_LAND_MOBILE      0b0110
#define MSISDN_NPI_ISDN_MOBILE      0b0111
#define MSISDN_NPI_PRIVATE      0b1110
#define MSISDN_NPI_RESERVED     0b1111
    Byte_t npi:4;
    /* Dialing Number           */
    struct {
        Byte_t lsb:4;
        Byte_t msb:4;
#define MSISDN_DIGIT_SIZE   10
    } digit[MSISDN_DIGIT_SIZE];
} msisdn_t;

/*
 * International Mobile Equipment Identity
 */
typedef imsi_t imei_t;

/*
 * Public Land Mobile Network identifier
 * PLMN = BCD encoding (Mobile Country Code + Mobile Network Code)
 */
typedef struct {
    Byte_t MCCdigit2:4;
    Byte_t MCCdigit1:4;
    Byte_t MNCdigit3:4;
    Byte_t MCCdigit3:4;
    Byte_t MNCdigit2:4;
    Byte_t MNCdigit1:4;
} plmn_t;

/*
 * Location Area Identification
 */
typedef struct {
    plmn_t plmn;    /* <MCC> + <MNC>    */
    lac_t lac;      /* Location Area Code   */
} lai_t;

/*
 * GPRS Routing Area Identification
 */
typedef struct {
    plmn_t plmn;    /* <MCC> + <MNC>    */
    lac_t lac;      /* Location Area Code   */
    rac_t rac;      /* Routing Area Code    */
} RAI_t;

/*
 * EPS Tracking Area Identification
 */
typedef struct {
    plmn_t plmn;    /* <MCC> + <MNC>    */
    tac_t tac;      /* Tracking Area Code   */
} tai_t;

/*
 * EPS Globally Unique MME Identity
 */
typedef struct {
    plmn_t plmn;    /* <MCC> + <MNC>    */
    UInt16_t MMEgid;    /* MME group identifier */
    UInt8_t MMEcode;    /* MME code     */
} gummei_t;

/*
 * EPS Globally Unique Temporary UE Identity
 */
typedef struct {
    gummei_t gummei;    /* Globally Unique MME Identity         */
    UInt32_t m_tmsi;    /* M-Temporary Mobile Subscriber Identity   */
} GUTI_t;

/* Checks PLMN validity */
#define PLMN_IS_VALID(plmn) (((plmn).MCCdigit1 &    \
                              (plmn).MCCdigit2 &    \
                              (plmn).MCCdigit3) != 0x0F)

/* Checks TAC validity */
#define TAC_IS_VALID(tac)   (((tac) != 0x0000) && ((tac) != 0xFFF0))

/* Checks TAI validity */
#define TAI_IS_VALID(tai)   (PLMN_IS_VALID((tai).plmn) &&   \
                             TAC_IS_VALID((tai).tac))
/*
 * A list of PLMNs
 */
#define PLMN_LIST_T(SIZE) struct {Byte_t n_plmns; plmn_t plmn[SIZE];}

/*
 * A list of TACs
 */
#define TAC_LIST_T(SIZE) struct {Byte_t n_tacs; TAC_t tac[SIZE];}

/*
 * A list of TAIs
 */
#define TAI_LIST_T(SIZE) struct {Byte_t n_tais; tai_t tai[SIZE];}

/*
 * User notification callback, executed whenever a change of data with
 * respect of network information (e.g. network registration and/or
 * location change, new PLMN becomes available) is notified by the
 * EPS Mobility Management sublayer
 */
typedef int (*emm_indication_callback_t) (Stat_t, tac_t, ci_t, AcT_t,
        const char *, size_t);

typedef enum eps_protocol_discriminator_e {
    /* Protocol discriminator identifier for EPS Mobility Management */
    EPS_MOBILITY_MANAGEMENT_MESSAGE =   0x7,

    /* Protocol discriminator identifier for EPS Session Management */
    EPS_SESSION_MANAGEMENT_MESSAGE =    0x2,
} eps_protocol_discriminator_t;

/****************************************************************************/
/********************  G L O B A L    V A R I A B L E S  ********************/
/****************************************************************************/

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

#endif /* __COMMONDEF_H__*/
