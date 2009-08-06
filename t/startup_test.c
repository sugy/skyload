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
static bool option_check_test(void);

int main(void) {
  if (allocation_test() == false)  
    return EXIT_FAILURE;
  if (multi_allocation_test() == false)
    return EXIT_FAILURE;
  if (option_check_test() == false)
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
  share->read_file_path = strdup("/path/to/file");

  if (!share->server || !share->create_query || !share->read_file_path)
    return false;

  free(share->server);
  free(share->create_query);
  free(share->read_file_path);

  share->server = NULL; 
  share->create_query = NULL; 
  share->read_file_path = NULL;
  
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

static bool option_check_test(void) {
  SKY_SHARE *share;
  FILE *redirect;

  /* redirect stderr to /dev/null to suppress the "intentional"
     error messages thrown by check_options() */
  if ((redirect = freopen("/dev/null", "w", stderr)) == NULL)
    return false;

  if ((share = sky_share_new()) == NULL)
    return false;

  /* specify skyload to auto-generate data */
  share->create_query = strdup("CREATE TABLE t1 (id int primary key)");
  share->insert_tmpl = strdup("INSERT INTO t1 VALUES (1)");
  share->columns = string_occurrence(share->insert_tmpl, "%");

  if (share->create_query == NULL || share->insert_tmpl == NULL)
    return false;

  /* this should fail due to lack of information */
  if (check_options(share) == true)
    return false;

  /* provide more information */
  if ((share->server = strdup("localhost")) == NULL)
    return false;

  share->nwrite = 100;

  /* this should still fail since there is no placeholder
     in the INSERT tmplate*/
  if (check_options(share) == true)
    return false;

  /* replace the magic number with a placeholder */
  free(share->insert_tmpl);
  share->insert_tmpl = strdup("INSERT INTO t1 VALUES (%seq)");
  share->columns = string_occurrence(share->insert_tmpl, "%");

  if (share->insert_tmpl == NULL)
    return false;

  /* there should now be sufficient information */
  if (check_options(share) == false)
    return false;

  /* provide a load file as well as the insert template */
  if ((share->load_file_path = strdup("/path/to/file")) == NULL)
    return false;

  /* skyload doesn't allow two sources of load data */
  if (check_options(share) == true)
    return false;

  /* try retesting by gettind rid of the INSERT template */
  free(share->insert_tmpl);
  share->insert_tmpl = NULL;

  if (check_options(share) == false)
    return false;

  /* specify read-load file */
  if ((share->read_file_path = strdup("/path/to/file")) == NULL)
    return false;
  
  if (check_options(share) == false)
    return false;

  fclose(redirect);
  sky_share_free(share);
  return true;
}
