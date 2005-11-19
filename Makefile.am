AM_CPPFLAGS= -I$(srcdir)/knumber -D_ISOC99_SOURCE $(all_includes)

SUBDIRS = knumber

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kcalc.la

kcalc_la_SOURCES = kcalc.cpp kcalc_button.cpp kcalc_const_button.cpp \
			kcalc_const_menu.cpp kcalc_core.cpp kcalcdisplay.cpp \
			dlabel.cpp stats.cpp colors.ui general.ui \
			constants.ui kcalc_settings.kcfgc


METASOURCES = AUTO

noinst_HEADERS  = kcalc.h kcalc_button.h kcalc_const_button.h \
			kcalc_const_menu.h kcalc_core.h kcalcdisplay.h \
			kcalctype.h dlabel.h stats.h

kcalc_la_LDFLAGS = $(all_libraries) -module -avoid-version
kcalc_la_LIBADD = $(LIB_KDEUI) knumber/libknumber.la

KDE_ICON = kcalc

xdg_apps_DATA = kcalc.desktop
kde_kcfg_DATA = kcalc.kcfg

rcdir = $(kde_datadir)/kcalc
rc_DATA = kcalcui.rc

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcalc.pot

updatedir = $(kde_datadir)/kconf_update
update_DATA = kcalcrc.upd

DOXYGEN_EMPTY = YES
include $(top_srcdir)/admin/Doxyfile.am

