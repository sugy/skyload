AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = t

bin_PROGRAMS = skyload

skyload_SOURCES = \
	skyload.c \
	utils.c \
	options.c \
	generator.c

skyload_CFLAGS  = $(AM_CFLAGS) -Wall
skyload_LDFLAGS = $(LIBDRIZZLE) -lpthread
