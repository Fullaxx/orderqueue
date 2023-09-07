#!/bin/bash

set -e

CFLAGS="-Wall -ansi -D_GNU_SOURCE"
OPTCFLAGS="${CFLAGS} -O2"
DBGCFLAGS="${CFLAGS} -ggdb3 -DDEBUG"

rm -f *.dbg

gcc ${DBGCFLAGS} -DORDERQUEUE_LONG  -DASCENDING -DDEBUG_INSERT test_random.c orderqueue.c -o random_long_ascending.dbg
gcc ${DBGCFLAGS} -DORDERQUEUE_LONG -DDESCENDING -DDEBUG_INSERT test_random.c orderqueue.c -o random_long_descending.dbg

gcc ${DBGCFLAGS} -DORDERQUEUE_DOUBLE  -DASCENDING -DDEBUG_INSERT test_random.c orderqueue.c -o random_dbl_ascending.dbg
gcc ${DBGCFLAGS} -DORDERQUEUE_DOUBLE -DDESCENDING -DDEBUG_INSERT test_random.c orderqueue.c -o random_dbl_descending.dbg

gcc ${DBGCFLAGS} -DORDERQUEUE_LONG  -DASCENDING -DDEBUG_INSERT test_dupe_rejection.c orderqueue.c -o test_dupe_rejection_asc.dbg
gcc ${DBGCFLAGS} -DORDERQUEUE_LONG -DDESCENDING -DDEBUG_INSERT test_dupe_rejection.c orderqueue.c -o test_dupe_rejection_dsc.dbg
