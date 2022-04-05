cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "22")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "90")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(kcalc VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.90.0")

find_package (ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMMarkAsTest)
include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

## Generate header with version number
ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX KCALC
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kcalc_version.h"
)

find_package (Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat)
endif()

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Crash
    Config
    ConfigWidgets
    GuiAddons
    I18n
    Notifications
    XmlGui
)

find_package(KF5DocTools ${KF5_MIN_VERSION})
set_package_properties(KF5DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(GMP)
set_package_properties(GMP PROPERTIES
    DESCRIPTION "The GNU Multiple Precision Arithmetic Library"
    URL "https://gmplib.org/"
    TYPE REQUIRED
    PURPOSE "Required for building KCalc."
)

find_package(MPFR)
set_package_properties(MPFR PROPERTIES
    DESCRIPTION "The GNU Multiple Precision Floating-Point Reliable Library"
    URL "https://www.mpfr.org/"
    TYPE REQUIRED
    PURPOSE "Required for building KCalc."
)

include(CheckTypeSize)
include(CheckIncludeFiles)

check_include_files(ieeefp.h     HAVE_IEEEFP_H)
check_type_size("signed long"    SIZEOF_SIGNED_LONG)
check_type_size("unsigned long"  SIZEOF_UNSIGNED_LONG)

configure_file(config-kcalc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcalc.h )
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055800)

add_subdirectory(knumber)
add_executable(kcalc)

target_sources(kcalc PRIVATE
   kcalc.cpp 
   bitbutton.cpp
   kcalc_bitset.cpp
   kcalc_button.cpp 
   kcalc_const_button.cpp 
   kcalc_const_menu.cpp 
   kcalc_core.cpp 
   kcalcdisplay.cpp 
   kcalc_statusbar.cpp
   stats.cpp 
   kcalc.qrc
   kcalchistory.cpp
   stats.cpp
   kcalc_const_menu.h
   kcalc_core.h
   kcalc_bitset.h
   bitbutton.h
   kcalc.h
   kcalc_button.h
   stats.h
   kcalc_const_button.h
   kcalc_statusbar.h
   kcalcdisplay.h
   kcalchistory.h
   kcalc.qrc )


ki18n_wrap_ui(kcalc
   kcalc.ui
   constants.ui
   colors.ui
   fonts.ui
   general.ui)

kconfig_add_kcfg_files(kcalc kcalc_settings.kcfgc )

# Sets the icon on Windows and OSX
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*.png")
ecm_add_app_icon(kcalc ICONS ${ICONS_SRCS})

# Conditionally install icons for Linux as they may not be provided by the user theme
option(INSTALL_ICONS "Install icons" OFF)
if (INSTALL_ICONS)
    include(ECMInstallIcons)
    ecm_install_icons( ICONS ${ICONS_SRCS} DESTINATION ${KDE_INSTALL_ICONDIR} )
endif()


target_link_libraries(kcalc
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Widgets
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::GuiAddons
    KF5::I18n
    KF5::Notifications
    KF5::XmlGui
    KF5::Crash
    knumber
)

set_target_properties(kcalc PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist.in)

install(TARGETS kcalc  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS org.kde.kcalc.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install( FILES org.kde.kcalc.appdata.xml   DESTINATION ${KDE_INSTALL_METAINFODIR})
install( FILES kcalc.kcfg  DESTINATION ${KDE_INSTALL_KCFGDIR})
install( FILES kcalcrc.upd  DESTINATION ${KDE_INSTALL_DATADIR}/kconf_update)

ki18n_install(po)
if (KF5DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
