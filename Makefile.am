
bin_PROGRAMS = kcalc

kcalc_SOURCES = kcalc.cpp kcalc_core.cpp dlabel.cpp stats.cpp \
	optiondialog.cpp
METASOURCES = kcalc.moc configdlg.moc fontdlg.moc dlabel.moc \
	optiondialog.moc

noinst_HEADERS  = kcalc.h configdlg.h fontdlg.h dlabel.h stats.h kcalctype.h

INCLUDES= $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   = $(LIB_KDEUI) -lm

pics_DATA = kcalclogo.png
picsdir = $(kde_datadir)/kcalc/pics

KDE_ICON = kcalc

apps_DATA = kcalc.desktop
appsdir = $(kde_appsdir)/Utilities

EXTRA_DIST = $(pics_DATA) $(apps_DATA) 

messages:
	$(XGETTEXT) $(kcalc_SOURCES) -o $(podir)/kcalc.pot

