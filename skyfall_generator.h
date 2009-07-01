/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#ifndef __SKYFALL_GENERATOR_H__
#define __SKYFALL_GENERATOR_H__

/* analyze the given INSERT template query */
void analyze_insert_template(SKYFALL_SHARE *share);

/* creates the next INSERT query for the given worker object.
   on success, the return value of this function is the length
   of the generated query and 0 on failure */
size_t get_next_insert_query(SKYFALL_WORKER *worker, char *buffer,
                             size_t buflen);

#endif