
bin_PROGRAMS = kcalc

kcalc_SOURCES = kcalc.cpp kcalc_core.cpp configdlg.cpp fontdlg.cpp \
	dlabel.cpp stats.cpp
METASOURCES = kcalc.moc configdlg.moc fontdlg.moc dlabel.moc

noinst_HEADERS  = kcalc.h configdlg.h fontdlg.h dlabel.h stats.h kcalctype.h

INCLUDES= $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   = $(LIB_KDEUI) -lm

SUBDIRS = doc

pics_DATA = kcalclogo.xpm
picsdir = $(kde_datadir)/kcalc/pics

kde_icon_DATA = kcalc.xpm

apps_DATA = kcalc.kdelnk
appsdir = $(kde_appsdir)/Utilities

EXTRA_DIST = $(pics_DATA) $(kde_icon_DATA) $(apps_DATA) mini-kcalc.xpm

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kcalc.xpm  $(kde_minidir)/kcalc.xpm

uninstall-local:
	rm -f $(MINIDIR)/kcalc.xpm

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kcalc_SOURCES) && mv messages.po ../po/kcalc.pot

