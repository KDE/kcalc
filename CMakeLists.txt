kde4_header()

configure_file(config-kcalc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcalc.h )

add_subdirectory( knumber ) 

include_directories( ${CMAKE_SOURCE_DIR}/kcalc/knumber  )


########### next target ###############
set(libknumber_la_SRCS  
   ${CMAKE_SOURCE_DIR}/kcalc/knumber/knumber.cpp 
   ${CMAKE_SOURCE_DIR}/kcalc/knumber/knumber_priv.cpp
   )

set(kcalc_KDEINIT_SRCS ${libknumber_la_SRCS}
   kcalc.cpp 
   kcalc_button.cpp 
   kcalc_const_button.cpp 
   kcalc_const_menu.cpp 
   kcalc_core.cpp 
   kcalcdisplay.cpp 
   dlabel.cpp 
   stats.cpp )

kde4_automoc(${kcalc_KDEINIT_SRCS})

kde4_add_ui3_files(kcalc_KDEINIT_SRCS colors.ui general.ui constants.ui )

kde4_add_kcfg_files(kcalc_KDEINIT_SRCS kcalc_settings.kcfgc )

kde4_add_kdeinit_executable( kcalc ${kcalc_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcalc  ${KDE4_KDEUI_LIBS} ${GMP_LIBRARIES})

install_targets(${LIB_INSTALL_DIR} kdeinit_kcalc )

target_link_libraries( kcalc kdeinit_kcalc )
install_targets(/bin kcalc )

########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcalc.desktop )
install_files( ${KCFG_INSTALL_DIR} FILES kcalc.kcfg )
install_files( ${DATA_INSTALL_DIR}/kcalc FILES kcalcui.rc )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES kcalcrc.upd )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS= -I$(srcdir)/knumber -D_ISOC99_SOURCE $(all_includes)
#
#SUBDIRS = knumber
#
#bin_PROGRAMS =
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = kcalc.la
#
#kcalc_la_SOURCES = kcalc.cpp kcalc_button.cpp kcalc_const_button.cpp \
#			kcalc_const_menu.cpp kcalc_core.cpp kcalcdisplay.cpp \
#			dlabel.cpp stats.cpp colors.ui general.ui \
#			constants.ui kcalc_settings.kcfgc
#
#
#METASOURCES = AUTO
#
#noinst_HEADERS  = kcalc.h kcalc_button.h kcalc_const_button.h \
#			kcalc_const_menu.h kcalc_core.h kcalcdisplay.h \
#			kcalctype.h dlabel.h stats.h
#
#kcalc_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kcalc_la_LIBADD = $(LIB_KDEUI) knumber/libknumber.la
#
#KDE_ICON = kcalc
#
#xdg_apps_DATA = kcalc.desktop
#kde_kcfg_DATA = kcalc.kcfg
#
#rcdir = $(kde_datadir)/kcalc
#rc_DATA = kcalcui.rc
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcalc.pot
#
#updatedir = $(kde_datadir)/kconf_update
#update_DATA = kcalcrc.upd
#
#DOXYGEN_EMPTY = YES
#include $(top_srcdir)/admin/Doxyfile.am
#
