project(kcalc)

check_include_files(ieeefp.h      HAVE_IEEEFP_H)
check_function_exists(isinf      HAVE_FUNC_ISINF)
check_function_exists(sinl HAVE_L_FUNCS)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)

configure_file(config-kcalc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcalc.h )

ADD_DEFINITIONS( -D_ISOC99_SOURCE ) # added manually


include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/knumber  )


########### next target ###############
# Needs absolute paths due to the test program for knumber
set(libknumber_la_SRCS  
   ${CMAKE_SOURCE_DIR}/kcalc/knumber/knumber.cpp 
   ${CMAKE_SOURCE_DIR}/kcalc/knumber/knumber_priv.cpp
   )

add_subdirectory( knumber )

set(kcalc_KDEINIT_SRCS ${libknumber_la_SRCS}
   kcalc.cpp 
   kcalc_bitset.cpp
   kcalc_button.cpp 
   kcalc_const_button.cpp 
   kcalc_const_menu.cpp 
   kcalc_core.cpp 
   kcalcdisplay.cpp 
   dlabel.cpp 
   stats.cpp )

kde4_automoc(${kcalc_KDEINIT_SRCS})

kde4_add_ui3_files(kcalc_KDEINIT_SRCS general.ui )

kde4_add_ui_files(kcalc_KDEINIT_SRCS constants.ui colors.ui)

kde4_add_kcfg_files(kcalc_KDEINIT_SRCS kcalc_settings.kcfgc )

kde4_add_kdeinit_executable( kcalc ${kcalc_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcalc  ${KDE4_KDEUI_LIBS} ${GMP_LIBRARIES})

install(TARGETS kdeinit_kcalc  DESTINATION ${LIB_INSTALL_DIR})

target_link_libraries( kcalc kdeinit_kcalc )
install(TARGETS kcalc  DESTINATION ${BIN_INSTALL_DIR} )

########### install files ###############

install( FILES kcalc.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES kcalc.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kcalcui.rc  DESTINATION ${DATA_INSTALL_DIR}/kcalc)
install( FILES scienceconstants.xml  DESTINATION ${DATA_INSTALL_DIR}/kcalc)
install( FILES kcalcrc.upd  DESTINATION ${DATA_INSTALL_DIR}/kconf_update)

kde4_install_icons( ${ICON_INSTALL_DIR}   )


