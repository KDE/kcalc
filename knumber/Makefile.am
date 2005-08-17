AM_CPPFLAGS=-D_GNU_SOURCE -D_ISOC99_SOURCE $(all_includes)

SUBDIRS = tests

bin_PROGRAMS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES = libknumber.la

libknumber_la_SOURCES = knumber.cpp knumber_priv.cpp

METASOURCES = AUTO

noinst_HEADERS  = knumber.h knumber_priv.h

kcalc_la_LIBADD = libknumber.la $(LIB_KDEUI) $(LIBGMP) -lm

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcalc.pot

updatedir = $(kde_datadir)/kconf_update

include ../../admin/Doxyfile.am
