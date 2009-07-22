/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "../skyload.h"

static bool allocation_test(void);
static bool multi_allocation_test(void);

int main(void) {
  if (allocation_test() == false)  
    return EXIT_FAILURE;
  if (multi_allocation_test() == false)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

static bool allocation_test(void) {
  SKY_WORKER *worker;
  SKY_SHARE *share;

  /* test allocator */
  if ((worker = sky_worker_new()) == NULL)
    return false;

  if ((share = sky_share_new()) == NULL)
    return false;
  
  /* test the deallocator. deliberatly free the allocated members
     and see if the deallocator will not double-free */
  share->server = strdup("localhost");
  share->create_query = strdup("CREATE TABLE t1 (id int primary key);");
  share->sql_file_path = strdup("/path/to/file");

  free(share->server);
  free(share->create_query);
  free(share->sql_file_path);

  share->server = NULL; 
  share->create_query = NULL; 
  share->sql_file_path = NULL;
  
  /* if this screws up, it will kill the test program */
  sky_share_free(share);
  sky_worker_free(worker);

  return true;
}

static bool multi_allocation_test(void) {
  SKY_WORKER **workers;
  SKY_SHARE *share;

  if ((share = sky_share_new()) == NULL)
    return false;

  share->concurrency = 8;

  if ((workers = create_workers(share)) == NULL)
    return false;

  destroy_workers(workers);
  sky_share_free(share);
  return true;
}
