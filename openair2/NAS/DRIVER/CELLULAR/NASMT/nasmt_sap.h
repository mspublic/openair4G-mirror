/***************************************************************************
                          nasmt_sap.h  -  description
                             -------------------
    copyright            : (C) 2002 by Eurecom
    email                : michelle.wetterwald@eurecom.fr
                           yan.moret@eurecom.fr
 ***************************************************************************

 ***************************************************************************/

#ifndef _NASMTD_SAP_H
#define _NASMTD_SAP_H


// RT-FIFO identifiers ** must be identical to Access Stratum as_sap.h and rrc_sap.h

#define RRC_DEVICE_GC          RRC_SAPI_UE_GCSAP
#define RRC_DEVICE_NT          RRC_SAPI_UE_NTSAP
#define RRC_DEVICE_DC_INPUT0   RRC_SAPI_UE_DCSAP_IN
#define RRC_DEVICE_DC_OUTPUT0  RRC_SAPI_UE_DCSAP_OUT

#define QOS_DEVICE_CONVERSATIONAL_INPUT  QOS_SAPI_CONVERSATIONAL_INPUT_MT
#define QOS_DEVICE_CONVERSATIONAL_OUTPUT QOS_SAPI_CONVERSATIONAL_OUTPUT_MT
#define QOS_DEVICE_STREAMING_INPUT       QOS_SAPI_STREAMING_INPUT_MT
#define QOS_DEVICE_STREAMING_OUTPUT      QOS_SAPI_STREAMING_OUTPUT_MT
#define QOS_DEVICE_INTERACTIVE_INPUT     QOS_SAPI_INTERACTIVE_INPUT_MT
#define QOS_DEVICE_INTERACTIVE_OUTPUT    QOS_SAPI_INTERACTIVE_OUTPUT_MT
#define QOS_DEVICE_BACKGROUND_INPUT      QOS_SAPI_BACKGROUND_INPUT_MT
#define QOS_DEVICE_BACKGROUND_OUTPUT     QOS_SAPI_BACKGROUND_OUTPUT_MT

//FIFO indexes in control blocks

#define GRAAL_DC_INPUT_SAPI	  0
#define GRAAL_DC_OUTPUT_SAPI	1
#define GRAAL_SAPI_CX_MAX	    2

#define GRAAL_GC_SAPI 		    0
#define GRAAL_NT_SAPI 		    1
#define GRAAL_CO_INPUT_SAPI	  2
#define GRAAL_IN_INPUT_SAPI	  3
#define GRAAL_ST_INPUT_SAPI	  4
#define GRAAL_BA_INPUT_SAPI	  5
#define GRAAL_CO_OUTPUT_SAPI	6
#define GRAAL_IN_OUTPUT_SAPI	7
#define GRAAL_ST_OUTPUT_SAPI	8
#define GRAAL_BA_OUTPUT_SAPI	9
#define GRAAL_SAPI_MAX		   10


#define GRAAL_QOS_CONVERSATIONAL UMTS_TRAFFIC_CONVERSATIONAL
#define GRAAL_QOS_STREAMING 	   UMTS_TRAFFIC_STREAMING
#define GRAAL_QOS_INTERACTIVE    UMTS_TRAFFIC_INTERACTIVE
#define GRAAL_QOS_BACKGROUND	   UMTS_TRAFFIC_BACKGROUND


#endif



