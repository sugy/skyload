# Skyfall Autoconf Script

AC_PREREQ(2.59)
AC_INIT(skyload, 0.5.0, dev@torum.net)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(skyload.c)
AC_CONFIG_HEADER([config/config.h])
AC_CONFIG_MACRO_DIR([config/m4])

AC_PROG_CC
AM_PROG_CC_C_O

AM_INIT_AUTOMAKE(-Wall -Werror nostdinc subdir-objects)

AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_C_CONST

AC_CHECK_FUNCS([strdup])
AC_FUNC_MALLOC

AC_SEARCH_LIBS(pthread)

CC="${CC} -std=gnu99"

#--------------------------------------------------------------------
# Check for libdrizzle
#--------------------------------------------------------------------

AC_LIB_HAVE_LINKFLAGS(drizzle,,
[#include <libdrizzle/drizzle.h>],
[
  const char *version= drizzle_version()
])
AS_IF([test x$ac_cv_libdrizzle = xno],
      AC_MSG_ERROR([libdrizzle is required for skyload]))

AC_DEFINE([LICENSE],[BSD],[skyload is distributed under the BSD license])
AC_HEADER_STDBOOL
AC_CONFIG_FILES(Makefile)

AC_OUTPUT(t/Makefile)
