AM_CPPFLAGS=-D_GNU_SOURCE -D_ISOC99_SOURCE $(all_includes)

bin_PROGRAMS =
lib_LTLIBRARIES = 
kdeinit_LTLIBRARIES = kcalc.la

kcalc_la_SOURCES = kcalc.cpp kcalc_core.cpp dlabel.cpp stats.cpp \
	           optiondialog.cpp

METASOURCES = AUTO

noinst_HEADERS  = kcalc.h dlabel.h stats.h kcalctype.h

kcalc_la_LDFLAGS = $(all_libraries) -module -avoid-version
kcalc_la_LIBADD = $(LIB_KDEUI) -lm

KDE_ICON = kcalc

xdg_apps_DATA = kcalc.desktop

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcalc.pot
