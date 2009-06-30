/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyfall.h"

void analyze_insert_template(SKYFALL_SHARE *share) {
  assert(share);

  /* calculate the number of columns we need to work with */
  share->columns = string_occurrence(share->insert_tmpl, "$");
}

size_t get_next_insert_query(SKYFALL_WORKER *worker, char *buffer,
                             size_t buflen) {
  return 0;
}
