cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "24")
set (RELEASE_SERVICE_VERSION_MINOR "11")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(kcalc VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")

find_package (ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMMarkAsTest)
include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(KDEGitCommitHooks)
include(KDEClangFormat)
include(ECMDeprecationSettings)
include(ECMAddTests)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

## Generate header with version number
ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX KCALC
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/kcalc_version.h"
)

find_package (Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Crash
    ColorScheme
    Config
    ConfigWidgets
    GuiAddons
    I18n
    Notifications
    XmlGui
)

find_package(KF6DocTools ${KF_MIN_VERSION})
set_package_properties(KF6DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_SOURCE_DIR}/cmake/modules)

ecm_set_disabled_deprecation_versions(QT 6.7  KF 6.5.0)

if(CMAKE_BUILD_TYPE MATCHES "Release")
   add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
add_subdirectory(knumber)
add_subdirectory(src)

# Sets the icon on Windows and OSX
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*.png")
ecm_add_app_icon(kcalc ICONS ${ICONS_SRCS})

# Conditionally install icons for Linux as they may not be provided by the user theme
option(INSTALL_ICONS "Install icons" OFF)
if (INSTALL_ICONS)
    include(ECMInstallIcons)
    ecm_install_icons( ICONS ${ICONS_SRCS} DESTINATION ${KDE_INSTALL_ICONDIR} )
endif()

install(TARGETS kcalc  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############
install( FILES kcalcrc.upd  DESTINATION ${KDE_INSTALL_DATADIR}/kconf_update)
install( DIRECTORY DESTINATION "${KDE_INSTALL_FULL_DATAROOTDIR}/kglobalaccel" )
install(
    CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink \"${KDE_INSTALL_FULL_APPDIR}/org.kde.kcalc.desktop\" \"\$ENV{DESTDIR}${KDE_INSTALL_FULL_DATAROOTDIR}/kglobalaccel/org.kde.kcalc.desktop\")"
)

if(BUILD_TESTING)
   add_subdirectory(autotests)
   add_subdirectory(appiumtests)
endif()

ki18n_install(po)
if (KF6DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
