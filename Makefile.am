AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = t

bin_PROGRAMS = skyload

skyload_SOURCES = \
	skyload.c \
	utils.c \
	options.c \
	generator.c

noinst_HEADERS= \
	skyload.h \
	generator.h

EXTRA_DIST = \
	t/test.sql

skyload_CFLAGS  = $(AM_CFLAGS) -Wall
skyload_LDFLAGS = $(LIBDRIZZLE) -lpthread
