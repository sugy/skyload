/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "../skyfall.h"

static bool occurrence_test(void);

int main(void) {
  if (occurrence_test() == false)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

static bool occurrence_test(void) {
  const char *haystack1 = "dance dance dance!!";

  if (string_occurrence(haystack1, "dance") != 3)
    return false;

  if (string_occurrence(haystack1, "swing") > 0)
    return false;

  if (string_occurrence(haystack1, "dance  dance") > 0)
    return false;

  const char *haystack2 = "CREATE TABLE t1 (id int primary key, entry text);";

  if (string_occurrence(haystack2, "primary key") != 1)
    return false;

  if (string_occurrence(haystack2, "CREATE") != 1)
    return false;

  const char *haystack3 = "hi\nmy\nname\nis\nskyfall\n";

  if (string_occurrence(haystack3, "hi my") > 0)
    return false;

  if (string_occurrence(haystack3, "hi\nmy") != 1)
    return false;

  return true;
}
