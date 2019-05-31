cmake_minimum_required(VERSION 3.5)

# KDE Application Version, managed by release script
set (KDE_APPLICATIONS_VERSION_MAJOR "19")
set (KDE_APPLICATIONS_VERSION_MINOR "07")
set (KDE_APPLICATIONS_VERSION_MICRO "70")
set (KDE_APPLICATIONS_VERSION "${KDE_APPLICATIONS_VERSION_MAJOR}.${KDE_APPLICATIONS_VERSION_MINOR}.${KDE_APPLICATIONS_VERSION_MICRO}")
project(kcalc VERSION ${KDE_APPLICATIONS_VERSION})

set(QT_MIN_VERSION "5.9.0")
set(KF5_MIN_VERSION "5.46.0")

find_package (ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMMarkAsTest)
include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMSetupVersion)

## Generate header with version number
ecm_setup_version(${KDE_APPLICATIONS_VERSION} VARIABLE_PREFIX KCALC
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kcalc_version.h"
)

find_package (Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Crash
    Config
    ConfigWidgets
    DocTools
    GuiAddons
    I18n
    Init
    Notifications
    XmlGui
)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_SOURCE_DIR}/cmake/modules)
kde_enable_exceptions()

find_package(GMP)
set_package_properties(GMP PROPERTIES
    DESCRIPTION "The GNU Multiple Precision Arithmetic Library"
    URL "https://gmplib.org/"
    TYPE REQUIRED
    PURPOSE "Required for building KCalc."
)

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
   kcalc_statusbar.cpp
   stats.cpp )

ki18n_wrap_ui(kcalc_KDEINIT_SRCS
   kcalc.ui
   constants.ui
   colors.ui
   fonts.ui
   general.ui)

kconfig_add_kcfg_files(kcalc_KDEINIT_SRCS kcalc_settings.kcfgc )

# Sets the icon on Windows and OSX
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*.png")
ecm_add_app_icon(kcalc_KDEINIT_SRCS ICONS ${ICONS_SRCS})

kf5_add_kdeinit_executable( kcalc ${kcalc_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcalc
    Qt5::Core
    Qt5::Widgets
    KF5::ConfigWidgets
    KF5::GuiAddons
    KF5::I18n
    KF5::Notifications
    KF5::XmlGui
    KF5::Crash
    ${GMP_LIBRARIES}
    ${MPFR_LIBRARIES}
)

install(TARGETS kdeinit_kcalc ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kcalc kdeinit_kcalc )
install(TARGETS kcalc  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS org.kde.kcalc.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install( FILES org.kde.kcalc.appdata.xml   DESTINATION ${KDE_INSTALL_METAINFODIR})
install( FILES kcalc.kcfg  DESTINATION ${KDE_INSTALL_KCFGDIR})
install( FILES kcalcui.rc  DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kcalc)
install( FILES scienceconstants.xml  DESTINATION ${KDE_INSTALL_DATADIR}/kcalc)
install( FILES kcalcrc.upd  DESTINATION ${KDE_INSTALL_DATADIR}/kconf_update)

add_subdirectory(doc)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
