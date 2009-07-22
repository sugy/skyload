/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "../generator.h"

static bool sky_list_test(void);

int main(void) {
  if (sky_list_test() == false)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

static bool sky_list_test(void) {
  SKY_LIST *list;
  SKY_LIST_NODE *curr;
  char buffer[1024];

  if ((list = sky_list_new()) == NULL)
    return false;

  for (int i = 0; i < 4000; i++) {
    sprintf(buffer, "%d", i);
    if (!sky_list_push(list, buffer, strlen(buffer))) {
      sky_list_free(list);
      return EXIT_FAILURE;
    }
  }

  curr = list->head;
  for (int i = 0; i < 4000; i++) {
    sprintf(buffer, "%d", i);
    if (strncmp(buffer, curr->data, curr->length)) {
      sky_list_free(list);
      return EXIT_FAILURE;
    }
    curr = curr->next;
  }
  sky_list_free(list);
  return true;
}
