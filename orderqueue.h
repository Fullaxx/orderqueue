/*
	A numerically ordered linked-list queue written in ANSI C
	Copyright (C) 2023 Brett Kuskie <fullaxx@gmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __ORDERQUEUE_H__
#define __ORDERQUEUE_H__

#if defined(ORDERQUEUE_DOUBLE)

typedef double oqtype_t;
#define OQFMT "%f"

#elif defined(ORDERQUEUE_LONG)

typedef long oqtype_t;
#define OQFMT "%ld"

#else

#error "define ORDERQUEUE_DOUBLE or ORDERQUEUE_LONG"

#endif

typedef struct orderqueue_node {
	void *pkg;
	oqtype_t num;
	struct orderqueue_node *lower;
	struct orderqueue_node *higher;
} oqnode_t;

int pipeline_insert(void *, oqtype_t, int);
long pipeline_count(void);
oqtype_t pipeline_span(void);
oqnode_t *pipeline_pop(int);
int print_num(oqnode_t *, char *, int);
void destroy_orphan(oqnode_t *);

#define OQ_LOWEST (0)
#define OQ_HIGHEST (1)

#endif
