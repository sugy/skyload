check_PROGRAMS = startup_test connection_test string_test generator_test

startup_test_SOURCES = startup_test.c ../skyfall_utils.c ../skyfall_options.c
startup_test_CFLAGS  = $(AM_CFLAGS)
startup_test_LDFLAGS = $(LIBDRIZZLE)

connection_test_SOURCES = connection_test.c ../skyfall_utils.c ../skyfall_options.c
connection_test_CFLAGS  = $(AM_CFLAGS)
connection_test_LDFLAGS = $(LIBDRIZZLE)

string_test_SOURCES = string_test.c ../skyfall_utils.c
string_test_CFLAGS  = $(AM_CFLAGS)
string_test_LDFLAGS = $(LIBDRIZZLE)

generator_test_SOURCES = \
	generator_test.c \
	../skyfall_utils.c \
	../skyfall_generator.c

generator_test_CFLAGS  = $(AM_CFLAGS)
generator_test_LDFLAGS = $(LIBDRIZZLE)

test:
	make check

TESTS = $(check_PROGRAMS)
