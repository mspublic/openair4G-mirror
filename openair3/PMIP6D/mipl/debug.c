/*
 * $Id: debug.c,v 1.1.1.1 2008/04/23 13:21:04 nguyenhn Exp $
 *
 * This file is part of the MIPL Mobile IPv6 for Linux.
 * 
 * Author: Antti Tuominen <anttit@tcs.hut.fi>
 *
 * Copyright 2003-2005 Go-Core Project
 * Copyright 2003-2006 Helsinki University of Technology
 *
 * MIPL Mobile IPv6 for Linux is free software; you can redistribute
 * it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; version 2 of
 * the License.
 *
 * MIPL Mobile IPv6 for Linux is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MIPL Mobile IPv6 for Linux; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/uio.h>

#include "debug.h"

FILE *sdbg;

void dbgprint(const char *fname, const char *fmt, ...)
{
        char s[1024];
        va_list args;
 
        va_start(args, fmt);
        vsprintf(s, fmt, args);
	if (fname)
		fprintf(stderr, "%s: ", fname);
	fprintf(stderr, "%s", s);
        va_end(args);
}

void debug_print_buffer(const void *data, int len, const char *fname, 
			const char *fmt, ...)
{ 
	int i; 
	char s[1024];
        va_list args;
 
        va_start(args, fmt);
        vsprintf(s, fmt, args);
        fprintf(stderr, "%s: %s", fname, s);
        va_end(args);
	for (i = 0; i < len; i++) { 
		if (i % 16 == 0) fprintf(stderr, "\n%04x: ", i); 
		fprintf(stderr, "%02x ", ((unsigned char *)data)[i]); 
	} 
	fprintf(stderr, "\n\n"); 
	
}
