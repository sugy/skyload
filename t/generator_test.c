/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "../generator.h"

static bool sky_list_test(void);
static bool file_load_test(void);

int main(void) {
  if (sky_list_test() == false)
    return EXIT_FAILURE;
  if (file_load_test() == false)
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

bool file_load_test(void) {
  SKY_SHARE *share;

  if ((share = sky_share_new()) == NULL)
    return false;

  share->sql_file_path = strdup("test.sql");

  if (!preload_sql_file(share)) {
    sky_share_free(share);
    return false;
  }

  /* test by looping with the linked list size */
  SKY_LIST_NODE *curr = share->query_list->head;

  for (int i = 0; i < share->query_list->size; i++) {
    if (strlen(curr->data) != curr->length) {
      sky_list_free(share->query_list);
      sky_share_free(share);
      return false;
    }
    curr = curr->next;
  }

  curr = share->query_list->head;

  /* test by looking with a pointer */
  while (curr->next != NULL) {
    if (strlen(curr->data) != curr->length) {
      sky_list_free(share->query_list);
      sky_share_free(share);
      return false;
    }
    curr = curr->next;
  }

  sky_list_free(share->query_list);
  sky_share_free(share);
  return true;
}
