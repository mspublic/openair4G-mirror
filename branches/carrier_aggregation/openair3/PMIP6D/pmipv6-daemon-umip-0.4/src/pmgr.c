/*
 * $Id: pmgr.c.in 1.3 06/02/08 00:53:40+02:00 vnuorval@tcs.hut.fi $
 *
 * This file is part of the MIPL Mobile IPv6 for Linux.
 * 
 * Author: Antti Tuominen <anttit@tcs.hut.fi>
 *
 * Copyright 2005 GO-Core Project
 * Copyright 2005,2006 Helsinki University of Technology
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
 * 02111-1307 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <limits.h>
#include "policy.h"
#include "pmgr.h"

static void pmgr_init_defaults(struct pmgr_cb *lb)
{
	memset(lb, 0, sizeof(struct pmgr_cb));
	strcpy(lb->so_path, "[internal]");

	lb->best_iface = default_best_iface;
	lb->best_coa = default_best_coa;
	lb->max_binding_life = default_max_binding_life;
	lb->discard_binding = default_discard_binding;
	lb->use_bradv = default_use_bradv;
	lb->use_keymgm = default_use_keymgm;
	lb->accept_inet6_iface = default_accept_inet6_iface;
	lb->accept_ra = default_accept_ra;
	lb->best_ro_coa = default_best_ro_coa;
}

static void pmgr_init_entrypoints(struct pmgr_cb *ep, void *h)
{
	/* To keep -fstrict-aliasing happy */
	union f_un {
		int (*i) ();
		void *v;
	} func;

	func.v = dlsym(h, "best_iface");
	if (dlerror() == NULL)
		ep->best_iface = func.i;
	func.v = dlsym(h, "best_coa");
	if (dlerror() == NULL)
		ep->best_coa = func.i;
	func.v = dlsym(h, "max_binding_life");
	if (dlerror() == NULL)
		ep->max_binding_life = func.i;
	func.v = dlsym(h, "discard_binding");
	if (dlerror() == NULL)
		ep->discard_binding = func.i;
	func.v = dlsym(h, "use_bradv");
	if (dlerror() == NULL)
		ep->use_bradv = func.i;
	func.v = dlsym(h, "use_keymgm");
	if (dlerror() == NULL)
		ep->use_keymgm = func.i;
	func.v = dlsym(h, "accept_inet6_iface");
	if (dlerror() == NULL)
		ep->accept_inet6_iface = func.i;
	func.v = dlsym(h, "accept_ra");
	if (dlerror() == NULL)
		ep->accept_ra = func.i;
	func.v = dlsym(h, "best_ro_coa");
	if (dlerror() == NULL)
		ep->best_ro_coa = func.i;
}

int pmgr_init(char *libpath, struct pmgr_cb *lb)
{
	struct pmgr_cb *old;
	void *h;

	if (libpath == NULL) {
		pmgr_init_defaults(lb);
		return 0;
	}

	h = dlopen(libpath, RTLD_LAZY);

	if (dlerror())
		return -ENOENT;

	old = (struct pmgr_cb *)malloc(sizeof(*old));
	if (old == NULL)
		return -ENOMEM;

	memcpy(old, lb, sizeof(*old));
	lb->old = old;

	strncpy(lb->so_path, libpath, _POSIX_PATH_MAX);
	pmgr_init_entrypoints(lb, h);
	lb->handle = h;

	return 0;
}

int pmgr_close(struct pmgr_cb *lb)
{
	void *h = lb->handle;
	struct pmgr_cb *t;

	if (lb->old) {
		t = lb->old;
		lb->old = lb->old->old;
		memcpy(lb, t, sizeof(*lb));
		free(t);
	}

	if (h != NULL)
		return dlclose(h);

	return 0;
}
