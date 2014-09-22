/***************************************************************************
                          nasrg_sap.h  -  description
 ***************************************************************************
  Eurecom OpenAirInterface 2
  Copyright(c) 1999 - 2013 Eurecom

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
  Address      : Eurecom, 450 route des Chappes, 06410 Biot Sophia Antipolis, France
*******************************************************************************/
/*! \file nasmt_sap.h
* \brief SAP constants for OpenAirInterface CELLULAR version - RG
* \author  michelle.wetterwald, navid.nikaein, raymond.knopp, Lionel Gauthier
* \company Eurecom
* \email: michelle.wetterwald@eurecom.fr, raymond.knopp@eurecom.fr, navid.nikaein@eurecom.fr,  lionel.gauthier@eurecom.fr
*/
/*******************************************************************************/
#ifndef _NASRGD_SAP_H
#define _NASRGD_SAP_H


// RT-FIFO identifiers ** must be identical to Access Stratum as_sap.h and rrc_sap.h

#define RRC_DEVICE_GC                    RRC_SAPI_RG_GCSAP
#define RRC_DEVICE_NT                    RRC_SAPI_RG_NTSAP
#define RRC_DEVICE_DC_INPUT0             RRC_SAPI_RG_DCSAP0_IN
#define RRC_DEVICE_DC_INPUT1             RRC_SAPI_RG_DCSAP1_IN
#define RRC_DEVICE_DC_OUTPUT0            RRC_SAPI_RG_DCSAP0_OUT
#define RRC_DEVICE_DC_OUTPUT1            RRC_SAPI_RG_DCSAP1_OUT
//
//#define QOS_DEVICE_CONVERSATIONAL_INPUT  QOS_SAPI_CONVERSATIONAL_INPUT_RG
//#define QOS_DEVICE_CONVERSATIONAL_OUTPUT QOS_SAPI_CONVERSATIONAL_OUTPUT_RG

#define PDCP2NAS_FIFO 21
#define NAS2PDCP_FIFO 22

//FIFO indexes in control blocks
#define NAS_DC_INPUT_SAPI  0
#define NAS_DC_OUTPUT_SAPI 1
#define NAS_SAPI_CX_MAX    2

#define NAS_GC_SAPI         0
#define NAS_NT_SAPI         1
#define NAS_DRB_INPUT_SAPI  2  //NAS_CO_INPUT_SAPI
#define NAS_DRB_OUTPUT_SAPI 3  //NAS_CO_OUTPUT_SAPI
#define NAS_SAPI_MAX        4

//#define NAS_QOS_CONVERSATIONAL UMTS_TRAFFIC_CONVERSATIONAL
//

/* Defined in RRC
#define RRC_NAS_GC_IN   0
#define RRC_NAS_GC_OUT  1
#define RRC_NAS_NT_IN   2
#define RRC_NAS_NT_OUT  3
#define RRC_NAS_DC0_IN  4
#define RRC_NAS_DC0_OUT 5
#define RRC_NAS_DC1_IN  6
#define RRC_NAS_DC1_OUT 7
#define RRC_NAS_DC2_IN  8
#define RRC_NAS_DC2_OUT 9
*/
#endif



