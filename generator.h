/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#ifndef __SKYFALL_GENERATOR_H__
#define __SKYFALL_GENERATOR_H__

#include "skyload.h"

#define PLACEHOLDER_SEQ  "%seq"
#define PLACEHOLDER_RAND "%rand"
#define PLACEHOLDER_SEQ_LEN  4
#define PLACEHOLDER_RAND_LEN 5

#define DEFAULT_RAND_MOD 10000 
#define MAX_LOADABLE_QUERIES 100000

/* creates the next INSERT query for the given worker object.
   on success, the return value of this function is the length
   of the generated query and 0 on failure */
size_t next_insert_query(SKY_WORKER *worker, char *buffer, size_t buflen);

/* read the provided external SQL file and convert the content
   into skyload's internal representation. The internal representation
   is a singly linked list (SKY_LIST) of queries. */
bool preload_sql_file(SKY_SHARE *share);

#endif
