/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "../skyfall.h"

static bool allocation_test(void);
static bool multi_allocation_test(void);
static bool column_list_test(void);

int main(void) {
  if (allocation_test() == false)  
    return EXIT_FAILURE;
  if (multi_allocation_test() == false)
    return EXIT_FAILURE;
  if (column_list_test() == false)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

static bool allocation_test(void) {
  SKYFALL_WORKER *worker;
  SKYFALL_SHARE *share;

  /* test allocator */
  if ((worker = skyfall_worker_new()) == NULL)
    return false;

  if ((share = skyfall_share_new()) == NULL)
    return false;
  
  /* test the deallocator. deliberatly free the allocated members
     and see if the deallocator will not double-free */
  share->server = strdup("localhost");
  share->create_query = strdup("CREATE TABLE t1 (id int primary key);");
  share->select_query = strdup("SELECT * FROM t1;");

  free(share->server);
  free(share->create_query);
  free(share->select_query);

  share->server = NULL; 
  share->create_query = NULL; 
  share->select_query = NULL; 
  
  /* if this screws up, it will kill the test program */
  skyfall_share_free(share);
  skyfall_worker_free(worker);

  return true;
}

static bool multi_allocation_test(void) {
  SKYFALL_WORKER **workers;
  SKYFALL_SHARE *share;

  if ((share = skyfall_share_new()) == NULL)
    return false;

  share->concurrency = 8;

  if ((workers = create_workers(share)) == NULL)
    return false;

  destroy_workers(workers);
  skyfall_share_free(share);
  return true;
}

static bool column_list_test(void) {
  SKYFALL_COLUMN *head = skyfall_column_new();
  SKYFALL_COLUMN *curr = head;
  uint32_t iterations = 50, i = 0;

  head->type = COLUMN_SEQUENTIAL;
  head->length = 0;

  for (i = 1; i < iterations; i++) {
    if (skyfall_column_push(head, COLUMN_SEQUENTIAL, i) == false) {
      skyfall_column_free_all(head);
      return false;
    }
  }

  i = 0;

  while (curr != NULL) {
    if (curr->length != i) {
      skyfall_column_free_all(head);
      return false;
    }
    curr = curr->next;
    i++;
  }

  if (iterations != i) {
    skyfall_column_free_all(head);
    return false;
  }

  skyfall_column_free_all(head);
  return true;
}
