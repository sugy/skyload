/* 
 * Copyright (C) 2009 Toru Maesaka <dev@torum.net>
 * All Rights Reserved.
 *
 * Use and distribution of this program is licensed under the
 * BSD license. See the COPYING file for full text.
 */

#include "../skyload.h"

static bool occurrence_test(void);
static bool lowercase_test(void);

int main(void) {
  if (occurrence_test() == false)
    return EXIT_FAILURE;
  if (lowercase_test() == false)
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

  const char *haystack3 = "hi\nmy\nname\nis\nskyload\n";

  if (string_occurrence(haystack3, "hi my") > 0)
    return false;

  if (string_occurrence(haystack3, "hi\nmy") != 1)
    return false;

  return true;
}

static bool lowercase_test(void) {
  char buf[SKY_STRSIZ];

  const char *ans1 = "a man can be destroyed but not defeated.";
  const char *ans2 = "there's no one thing that is true. they're all true.";
  const char *ans3 = "$$$ a ^^^ b *** c (((( d +++ e @@@ f !@_(*][}{g";

  sprintf(buf, "A MAN can BE DESTROYED but NOT defeated.");
  sky_tolower(buf);

  if (strcmp(buf, ans1) != 0)
    return false;

  sprintf(buf, "TheRe's nO OnE thINg ThAt is TRUE. ThEy're ALL trUe.");
  sky_tolower(buf);

  if (strcmp(buf, ans2) != 0)
    return false;

  sprintf(buf, "$$$ A ^^^ B *** C (((( D +++ E @@@ F !@_(*][}{G");
  sky_tolower(buf);

  if (strcmp(buf, ans3) != 0)
    return false;

  return true;
}
