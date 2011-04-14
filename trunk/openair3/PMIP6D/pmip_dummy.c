/*! \file pmip_dummy.c
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
/*****************************************************************
* C Implementation: pmip_dummy
* Description:
* Author: Eurecom Institute <Huu-Nghia.Nguyen@eurecom.fr>, (C) 2008
* Contributor:
*   Lionel Gauthier
* Copyright: Eurecom Institute
******************************************************************/
#include <stdint.h>
#include <pthread.h>
#include "mipv6.h"
#include "debug.h"
#include "crypto.h"
#include "conf.h"
#define pmip_dummy(...) dbg("Dummy function\r\n")
struct mip6_config conf;
FILE *yyin;
