AM_CPPFLAGS=-D_GNU_SOURCE -D_ISOC99_SOURCE
bin_PROGRAMS = kcalc
lib_LTLIBRARIES = kdeinit_kcalc.la

kdeinit_kcalc_la_SOURCES = kcalc.cpp kcalc_core.cpp dlabel.cpp stats.cpp \
	           optiondialog.cpp

METASOURCES = AUTO

noinst_HEADERS  = kcalc.h dlabel.h stats.h kcalctype.h

INCLUDES= $(all_includes)
kdeinit_kcalc_la_LDFLAGS = $(all_libraries) -module -avoid-version
kdeinit_kcalc_la_LIBADD = $(LIB_KDEUI) -lm

kcalc_SOURCES = dummy.cpp
kcalc_LDADD = kdeinit_kcalc.la
kcalc_LDFLAGS = $(all_libraries) $(KDE_RPATH)

pics_DATA = 
picsdir = $(kde_datadir)/kcalc/pics

KDE_ICON = kcalc

apps_DATA = kcalc.desktop
appsdir = $(kde_appsdir)/Utilities

EXTRA_DIST = $(pics_DATA) $(apps_DATA) 

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcalc.pot

dummy.cpp:
	echo > dummy.cpp
