AM_CPPFLAGS=-D_GNU_SOURCE -D_ISOC99_SOURCE $(all_includes)

bin_PROGRAMS =
lib_LTLIBRARIES = 
kdeinit_LTLIBRARIES = kcalc.la

kcalc_la_SOURCES = kcalc.cpp kcalcdisplay.cpp dlabel.cpp \
		   stats.cpp colors.ui general.ui kcalc_settings.kcfgc

METASOURCES = AUTO

noinst_HEADERS  = kcalc.h kcalc_core.h kcalcdisplay.h dlabel.h stats.h kcalctype.h

kcalc_la_LDFLAGS = $(all_libraries) -module -avoid-version
kcalc_la_LIBADD = $(LIB_KDEUI) -lm

KDE_ICON = kcalc

xdg_apps_DATA = kcalc.desktop
kde_kcfg_DATA = kcalc.kcfg

rcdir = $(kde_datadir)/kcalc
rc_DATA = kcalcui.rc

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcalc.pot

updatedir = $(kde_datadir)/kconf_update
update_DATA = kcalcrc.upd
