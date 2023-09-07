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
#include <string.h>
#include <pthread.h>

#include "orderqueue.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#define PIPELOCK	pthread_mutex_lock(&mutex)
#define PIPEUNLOCK 	pthread_mutex_unlock(&mutex)

/**
	g_highest is a pointer to the highest element in the pipeline
	g_lowest is a pointer to the lowest element in the pipeline
	g_count is an updated count of elements in the pipeline

	     g_highest                                    g_lowest
	|----------------|    |----------------|    |----------------|
	| 1181.000000500 | -> | 1181.000000100 | -> | 1180.000000900 |
	|----------------|    |----------------|    |----------------|
*/
oqnode_t *g_highest = NULL;
oqnode_t *g_lowest = NULL;
unsigned long g_count = 0;

static void bail(char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(EXIT_FAILURE);
}

static void* new_oqnode(void)
{
	void *p = calloc(1, sizeof(oqnode_t));
	if(!p) { bail("calloc() failed!"); }
	return p;
}

/**
	Search our list for the numerically correct insertion point
	If the new number is higher, then return NULL
	Otherwise return the location that will precede our new number
*/
static oqnode_t* find_predecessor(oqtype_t num)
{
	oqnode_t *cursor;

	if(num >= g_highest->num) { return NULL; }

	cursor = g_highest;
	while(cursor->lower) {
		if(num >= cursor->lower->num) { break; }
		cursor = cursor->lower;
	}

	return cursor;
}

#define BAIL_INSERTION_SAFELY(p) { free(p); PIPEUNLOCK; return 0; }

/**
	Insert a data package into the pipeline
	Push all lower elements to the right
	asterisks are printed if we updated a global edge pointer
	ellipsis are printed otherwise
	return the number of elements inserted
*/
int pipeline_insert(void *pkg, oqtype_t num, int reject_dupes)
{
	int n = 0;
	char log[128];
	oqnode_t *cursor = NULL;
	oqnode_t *pre = NULL;
	oqnode_t *post = NULL;

	PIPELOCK;

	if(!g_highest) {
		/* Create the head element */
		cursor = g_highest = new_oqnode();
	} else {
		/* Find the numerically correct insertion point */
		pre = find_predecessor(num);
		cursor = new_oqnode();
		if(!pre) {
			/* If we are not allowing duplicates, reject them here */
			if((reject_dupes) && (g_highest) && (num == g_highest->num)) { BAIL_INSERTION_SAFELY(cursor) }

			/* cursor is higher than g_highest */
			post = g_highest;
			g_highest = cursor;
		} else {
			/* If we are not allowing duplicates, reject them here */
			if((reject_dupes) && (pre->lower) && (num == pre->lower->num)) { BAIL_INSERTION_SAFELY(cursor) }

			/* cursor is lower than pre */
			n += snprintf(&log[n], 128-n, "... ");
			n += snprintf(&log[n], 128-n, OQFMT, pre->num);
			n += snprintf(&log[n], 128-n, " -> ");
			post = pre->lower;
			pre->lower = cursor;
			cursor->higher = pre;
		}
	}

	if(!pre) { n += snprintf(&log[n], 128-n, "*** "); }
	n += snprintf(&log[n], 128-n, OQFMT, num);

	if(post) {
		cursor->lower = post;
		post->higher = cursor;
		n += snprintf(&log[n], 128-n, " -> ");
		n += snprintf(&log[n], 128-n, OQFMT, post->num);
		n += snprintf(&log[n], 128-n, " ...");
	} else {
		/* cursor is lowest */
		g_lowest = cursor;
		n += snprintf(&log[n], 128-n, " ***");
	}

	/* Fill in all the details */
	cursor->num = num;
	cursor->pkg = pkg;
	g_count++;

	PIPEUNLOCK;

#ifdef DEBUG_INSERT
	fprintf(stderr, "INSERT: %s\n", log);
#endif
	return 1;
}

/* How many elements in the pipeline? */
long pipeline_count(void)
{
	return g_count;
}

/* What is the span of the pipeline? */
oqtype_t pipeline_span(void)
{
	oqtype_t diff;

	if(!g_highest || !g_lowest) { return 0; }

	PIPELOCK;
	diff = g_highest->num - g_lowest->num;
	PIPEUNLOCK;

	return diff;
}

/**
	Pop an edge element without pipeline traversal
	HIGHEST: Return the first element in the pipeline, if any
	HIGHEST: Reset g_highest to second element in the pipeline, if any
	LOWEST : Return the last element in the pipeline, if any
	LOWEST : Reset g_lowest to the second-to-last element in the pipeline, if any
*/
oqnode_t* pipeline_pop(int which)
{
	oqnode_t *cursor = NULL;

	if((which != OQ_LOWEST) && (which != OQ_HIGHEST)) { return NULL; }

	PIPELOCK;
	if(which == OQ_HIGHEST) {
		if(g_highest) {
			cursor = g_highest;
			g_highest = g_highest->lower;				/* Update g_highest ptr */
			if(g_highest) { g_highest->higher = NULL; }	/* Now there is nothing higher */
			else { g_lowest = NULL; }					/* If there is nothing lower, update g_lowest */
			g_count--;
		}
	}
	if(which == OQ_LOWEST) {
		if(g_lowest) {
			cursor = g_lowest;
			g_lowest = g_lowest->higher;				/* Update g_lowest ptr */
			if(g_lowest) { g_lowest->lower = NULL; }	/* Now there is nothing lower */
			else { g_highest = NULL; }					/* If there is nothing higher, update g_highest */
			g_count--;
		}
	}
	PIPEUNLOCK;

	if(cursor) {
		/* cursor is now an orphan */
		cursor->higher = NULL;
		cursor->lower = NULL;
	}
	return cursor;
}

int print_num(oqnode_t *p, char *buf, int len)
{
	return snprintf(buf, len, OQFMT, p->num);
}

/**
	Any element that gets pipeline_pop()'d and handled
	should get passed to destroy_orphan() after usage for memory reclamation
*/
void destroy_orphan(oqnode_t *p)
{
	if(p) { free(p); }
}
