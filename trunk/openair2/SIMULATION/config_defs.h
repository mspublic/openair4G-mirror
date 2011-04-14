#ifndef __CONFIG_DEFS_H__
#define __CONFIG_DEFS_H__

#include <stdio.h>
#include <stdlib.h>
//#include <string.h>

//#include "openair_types.h"

#define MAX_ACTION_NAME_SIZE 100
#define MAX_SECTION_NAME_SIZE 100
#define MAX_LINE_SIZE 200
#define MAX_CFG_SECTIONS 10
 


typedef struct
{
	char ActionName[MAX_ACTION_NAME_SIZE];
	int	(*Func) (int);
} cfg_Action;


typedef struct
{
	char SectionName[MAX_SECTION_NAME_SIZE];
	void	(*Func) (FILE*, int);
} cfg_Section;


#endif /*__CONFIG_DEFS_H__*/












