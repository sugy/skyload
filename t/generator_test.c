/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "../skyfall_generator.h"

static bool analyzer_test(void);

int main(void) {
  if (analyzer_test() == false)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

static bool analyzer_test(void) {
  SKYFALL_COLUMN_LIST *list;
  SKYFALL_COLUMN_NODE *curr;
  SKYFALL_SHARE *share = skyfall_share_new();

  share->insert_tmpl = strdup("INSERT INTO t1 VALUES ($seq, $rand, $seq, $rand);");

  if (share->insert_tmpl == NULL)
    return false;
  
  list = analyze_insert_template(share);

  if (list == NULL)
    return false;

  if (list->size != 4)
    return false;

  curr = list->head;

  for (int i = 1; i <= share->columns; i++) {
    if (i % 2 == 0) {
      if (curr->type != COLUMN_RANDOM)
        return false;
    } else {
      if (curr->type != COLUMN_SEQUENTIAL)
        return false;
    }
    curr = curr->next;
  }

  skyfall_column_list_free(list);
  free(share->insert_tmpl);

  /* check if the analyzer can catch an invalid placeholder */
  share->insert_tmpl = strdup("INSERT INTO t1 VALUES ($seq, $boo, $foo);");

  /* this should fail */
  list = analyze_insert_template(share);

  if (list != NULL)
    return false;

  skyfall_column_list_free(list);
  skyfall_share_free(share);
  return true;
}
