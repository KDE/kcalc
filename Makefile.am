AM_CPPFLAGS=-D_GNU_SOURCE -D_ISOC99_SOURCE
bin_PROGRAMS = kcalc
lib_LTLIBRARIES = kcalc.la

kcalc_la_SOURCES = kcalc.cpp kcalc_core.cpp dlabel.cpp stats.cpp \
	           optiondialog.cpp

METASOURCES = AUTO

noinst_HEADERS  = kcalc.h dlabel.h stats.h kcalctype.h

INCLUDES= $(all_includes)
kcalc_la_LDFLAGS = $(all_libraries) -module -avoid-version
kcalc_la_LIBADD = $(LIB_KDEUI) -lm

kcalc_SOURCES = dummy.cpp
kcalc_LDADD = kcalc.la
kcalc_LDFLAGS = $(all_libraries) $(KDE_RPATH)

KDE_ICON = kcalc

xdg_apps_DATA = kcalc.desktop

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcalc.pot

dummy.cpp:
	echo > dummy.cpp
