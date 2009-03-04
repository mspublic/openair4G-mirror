/*****************************************************************
 * C Implementation: pmip_dummy
 * Description: 
 * Author: Eurecom Institute <Huu-Nghia.Nguyen@eurecom.fr>, (C) 2008
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

