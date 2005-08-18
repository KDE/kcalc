AM_CPPFLAGS=-D_GNU_SOURCE -D_ISOC99_SOURCE $(all_includes)

SUBDIRS = tests

bin_PROGRAMS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES = libknumber.la

libknumber_la_SOURCES = knumber.cpp knumber_priv.cpp
libknumber_la_LIBADD = $(LIBGMP) -lm

METASOURCES = AUTO

noinst_HEADERS  = knumber.h knumber_priv.h

include ../../admin/Doxyfile.am
