/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#ifndef __SKYFALL_GENERATOR_H__
#define __SKYFALL_GENERATOR_H__

#include "skyfall.h"

#define PLACEHOLDER_SEQ  "$seq"
#define PLACEHOLDER_RAND "$rand"
#define PLACEHOLDER_SEQ_LEN  4
#define PLACEHOLDER_RAND_LEN 5

#define DEFAULT_RAND_MOD 10000 

/* creates the next INSERT query for the given worker object.
   on success, the return value of this function is the length
   of the generated query and 0 on failure */
size_t next_insert_query(SKYFALL_WORKER *worker, char *buffer,
                         size_t buflen);

#endif
