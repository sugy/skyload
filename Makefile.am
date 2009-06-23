AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = t

bin_PROGRAMS   = skyfall 

skyfall_SOURCES = skyfall.c skyfall_utils.c skyfall_options.c
skyfall_CFLAGS  = $(AM_CFLAGS)
skyfall_LDFLAGS = $(LIBDRIZZLE)
