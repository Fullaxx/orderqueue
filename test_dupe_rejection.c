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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "orderqueue.h"
#include "package.h"

static void print_and_destroy(oqnode_t *p)
{
	char num[64];

	if(!p) { return; }
	print_num(p, &num[0], sizeof(num));
	printf("%s (%ld) {"OQFMT"}\n", num, pipeline_count(), pipeline_span());
	free(p->pkg);
	destroy_orphan(p);
}

static void add_to_orderqueue(oqtype_t num)
{
	dp_t *dp;

	/* Create a data package */
	dp = (dp_t *)malloc(sizeof(dp_t));
	dp->buf = (unsigned char *)"?";
	dp->len = 2;

	/* Insert data package into the pipeline */
	int n = pipeline_insert(dp, num, 1);
	if(n == 0) { printf("%ld not inserted!\n", num); }
}

int main(int argc, char *argv[])
{
	oqnode_t *p;

	add_to_orderqueue(41);
	add_to_orderqueue(41);
	add_to_orderqueue(42);
	add_to_orderqueue(42);
	add_to_orderqueue(41);
	add_to_orderqueue(43);
	add_to_orderqueue(43);
	add_to_orderqueue(42);
	add_to_orderqueue(41);

	printf("\n");

#ifdef ASCENDING
	while( (p = pipeline_pop(OQ_LOWEST)) ) {
		print_and_destroy(p);
	}
#endif

#ifdef DESCENDING
	while( (p = pipeline_pop(OQ_HIGHEST)) ) {
		print_and_destroy(p);
	}
#endif

	return 0;
}
