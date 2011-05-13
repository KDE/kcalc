project(kcalc)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4)
    include(KDE4Defaults)
    set(GMP_REQUIRED TRUE)
else()
    set(GMP_REQUIRED FALSE)
endif()

find_package(GMP)
macro_log_feature( GMP_FOUND "GMP" "The GNU Multiple Precision Arithmetic Library" "http://gmplib.org/" ${GMP_REQUIRED} "" "Required for building KCalc.")

if(NOT GMP_FOUND)
    if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
        macro_display_feature_log()
    else()
        return()
    endif()
endif(NOT GMP_FOUND)

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFiles)

check_include_files(ieeefp.h      HAVE_IEEEFP_H)
check_function_exists(isinf      HAVE_FUNC_ISINF)
check_type_size("signed long" SIZEOF_SIGNED_LONG)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)

configure_file(config-kcalc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcalc.h )

include_directories(${KDE4_INCLUDES})
include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/knumber ${GMP_INCLUDE_DIR} )

########### next target ###############
# Needs absolute paths due to the test program for knumber
set(libknumber_la_SRCS  
   ${kcalc_SOURCE_DIR}/knumber/knumber.cpp 
   ${kcalc_SOURCE_DIR}/knumber/knumber_priv.cpp
   )

add_subdirectory( knumber )
# add_subdirectory( tests )

set(kcalc_KDEINIT_SRCS ${libknumber_la_SRCS}
   kcalc.cpp 
   kcalc_bitset.cpp
   kcalc_button.cpp 
   kcalc_const_button.cpp 
   kcalc_const_menu.cpp 
   kcalc_core.cpp 
   kcalcdisplay.cpp 
   stats.cpp )

kde4_add_ui_files(kcalc_KDEINIT_SRCS
   kcalc.ui
   constants.ui
   colors.ui
   fonts.ui
   general.ui)

kde4_add_kcfg_files(kcalc_KDEINIT_SRCS kcalc_settings.kcfgc )

kde4_add_app_icon(kcalc_KDEINIT_SRCS "${KDE4_INSTALL_DIR}/share/icons/oxygen/*/apps/accessories-calculator.png")

kde4_add_kdeinit_executable( kcalc ${kcalc_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcalc ${QT_QTXML_LIBRARY} ${KDE4_KDEUI_LIBS} ${GMP_LIBRARIES})

install(TARGETS kdeinit_kcalc ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kcalc kdeinit_kcalc )
install(TARGETS kcalc  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS kcalc.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kcalc.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kcalcui.rc  DESTINATION ${DATA_INSTALL_DIR}/kcalc)
install( FILES scienceconstants.xml  DESTINATION ${DATA_INSTALL_DIR}/kcalc)
install( FILES kcalcrc.upd  DESTINATION ${DATA_INSTALL_DIR}/kconf_update)

kde4_install_icons( ${ICON_INSTALL_DIR}   )
