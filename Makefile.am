kcalc_LDFLAGS	= $(all_libraries)  $(KDE_RPATH)
kcalc_LDADD =  $(LIB_KDEUI) -lm
INCLUDES =	$(all_includes)

####### Files

bin_PROGRAMS  = kcalc

noinst_HEADERS	= kcalc.h dlabel.h stats.h kcalctype.h
kcalc_SOURCES	= kcalc.cpp kcalc_core.cpp dlabel.cpp stats.cpp optiondialog.cpp

METASOURCES = AUTO

KDE_ICON = kcalc

apps_DATA = kcalc.desktop
appsdir = $(kde_appsdir)/Utilities

pics_DATA = 
picsdir = $(kde_datadir)/kcalc/pics

EXTRA_DIST = $(pics_DATA) $(apps_DATA) 

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcalc.pot
