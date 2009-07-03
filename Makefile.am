AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = t

bin_PROGRAMS = skyfall 

skyfall_SOURCES = \
	skyfall.c \
	utils.c \
	options.c \
	generator.c

skyfall_CFLAGS  = $(AM_CFLAGS) -Wall
skyfall_LDFLAGS = $(LIBDRIZZLE) -lpthread
