
bin_PROGRAMS = kcalc

kcalc_SOURCES = kcalc.cpp kcalc_core.cpp configdlg.cpp fontdlg.cpp \
	dlabel.cpp stats.cpp
BUILTSOURCES = kcalc.moc configdlg.moc fontdlg.moc dlabel.moc
DISTCLEANFILES = $(BUILTSOURCES)

noinst_HEADERS  = kcalc.h configdlg.h fontdlg.h dlabel.h stats.h kcalctype.h

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = -lkdecore -lkdeui -lqt -lX11 -lXext -lm $(LIBSOCKET)

SUBDIRS = doc

pics_DATA = kcalclogo.xpm
picsdir = $(kde_datadir)/kcalc/pics

icon_DATA = kcalc.xpm
icondir = $(kde_icondir)

apps_DATA = kcalc.kdelnk
appsdir = $(kde_appsdir)/Utilities

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kcalc.xpm  $(kde_minidir)/kcalc.xpm

uninstall-local:
	rm -f $(MINIDIR)/kcalc.xpm

messages:

kcalc.cpp:kcalc.moc
kcalc.moc: kcalc.h
	$(MOC) kcalc.h -o kcalc.moc

configdlg.cpp:configdlg.moc
configdlg.moc: configdlg.h
	$(MOC) configdlg.h -o configdlg.moc

fontdlg.cpp:fontdlg.moc
fontdlg.moc: fontdlg.h
	$(MOC) fontdlg.h -o fontdlg.moc

dlabel.cpp:dlabel.moc
dlabel.moc: dlabel.h
	$(MOC) dlabel.h -o dlabel.moc



