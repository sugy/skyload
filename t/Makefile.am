check_PROGRAMS = startup_test

startup_test_SOURCES = startup_test.c ../skyfall_utils.c ../skyfall_options.c
startup_test_CFLAGS  = $(AM_CFLAGS)
startup_test_LDFLAGS = $(LIBDRIZZLE)

test:
	make check

TESTS = $(check_PROGRAMS)
