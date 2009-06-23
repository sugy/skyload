/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "skyfall.h"

int main(int argc, char **argv) {
  SKYFALL_SHARE *share;
  SKYFALL *skyfall;

  if (argc == 1)
    usage();

  /* This object is shared among all worker threads */
  if ((share = skyfall_share_new()) == NULL) {
    report_error("out of memory");
    return EXIT_FAILURE;
  }

  /* Get user options and set it to share */
  if (handle_options(share, argc, argv) == false)
    return EXIT_FAILURE;

  /* Check if the provided options make sense */
  if (check_options(share) == false)
    return EXIT_FAILURE;

  /* TODO: encapsulate this into create_workers() which will
           be capable of creating multiple workers */
  if ((skyfall = skyfall_new()) == NULL) {
    report_error("out of memory");
    return EXIT_FAILURE;
  }

  skyfall->share = share;

  skyfall_share_free(share);
  skyfall_free(skyfall);
  return EXIT_SUCCESS;
}
