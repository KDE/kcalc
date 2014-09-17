project(kcalc)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.2.0")

find_package (ECM 1.3.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)

find_package (Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)

find_package(KF5 REQUIRED COMPONENTS
    KDELibs4Support
)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_SOURCE_DIR}/cmake/modules)
kde_enable_exceptions()

find_package(GMP REQUIRED)
# macro_log_feature( GMP_FOUND "GMP" "The GNU Multiple Precision Arithmetic Library" "http://gmplib.org/" TRUE "" "Required for building KCalc.")

include(CheckTypeSize)
include(CheckIncludeFiles)

check_include_files(ieeefp.h     HAVE_IEEEFP_H)
check_type_size("signed long"    SIZEOF_SIGNED_LONG)
check_type_size("unsigned long"  SIZEOF_UNSIGNED_LONG)

configure_file(config-kcalc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcalc.h )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/knumber ${GMP_INCLUDE_DIR} )

########### next target ###############
# Needs absolute paths due to the test program for knumber
set(libknumber_la_SRCS  
	${kcalc_SOURCE_DIR}/knumber/knumber.cpp
	${kcalc_SOURCE_DIR}/knumber/knumber_error.cpp
	${kcalc_SOURCE_DIR}/knumber/knumber_float.cpp
	${kcalc_SOURCE_DIR}/knumber/knumber_fraction.cpp
	${kcalc_SOURCE_DIR}/knumber/knumber_integer.cpp
	${kcalc_SOURCE_DIR}/knumber/knumber_operators.cpp
)

add_subdirectory( knumber )
# add_subdirectory( tests )

set(kcalc_KDEINIT_SRCS ${libknumber_la_SRCS}
   kcalc.cpp 
   bitbutton.cpp
   kcalc_bitset.cpp
   kcalc_button.cpp 
   kcalc_const_button.cpp 
   kcalc_const_menu.cpp 
   kcalc_core.cpp 
   kcalcdisplay.cpp 
   stats.cpp )

qt5_wrap_ui(kcalc_KDEINIT_SRCS
   kcalc.ui
   constants.ui
   colors.ui
   fonts.ui
   general.ui)

kconfig_add_kcfg_files(kcalc_KDEINIT_SRCS kcalc_settings.kcfgc )

#kde4_add_app_icon(kcalc_KDEINIT_SRCS "${KDE4_INSTALL_DIR}/share/icons/oxygen/*/apps/accessories-calculator.png")

kf5_add_kdeinit_executable( kcalc ${kcalc_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcalc Qt5::Widgets KF5::KDELibs4Support ${GMP_LIBRARIES} ${MPFR_LIBRARIES})

install(TARGETS kdeinit_kcalc ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kcalc kdeinit_kcalc )
install(TARGETS kcalc  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS kcalc.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kcalc.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kcalcui.rc  DESTINATION ${KXMLGUI_INSTALL_DIR}/kcalc)
install( FILES scienceconstants.xml  DESTINATION ${DATA_INSTALL_DIR}/kcalc)
install( FILES kcalcrc.upd  DESTINATION ${DATA_INSTALL_DIR}/kconf_update)

add_subdirectory(doc)
