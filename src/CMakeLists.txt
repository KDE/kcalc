add_executable(kcalc)
target_sources(kcalc PRIVATE
   kcalc.cpp
   kcalc_bitbutton.cpp
   kcalc_bitset.cpp
   kcalc_button.cpp
   kcalc_const_button.cpp
   kcalc_const_menu.cpp
   kcalc_core.cpp
   kcalc_core_p.cpp
   kcalc_input_display.cpp
   kcalc_display_frame.cpp
   kcalc_parser.cpp
   kcalc_token.cpp
   kcalc_display.cpp
   kcalc_statusbar.cpp
   kcalc_stats.cpp
   kcalc_history.cpp
   kcalc_numeralsystem_view.cpp
   kcalc_const_menu.h
   kcalc_core.h
   kcalc_core_p.h
   kcalc_bitset.h
   kcalc_bitbutton.h
   kcalc.h
   kcalc_button.h
   kcalc_stats.h
   kcalc_const_button.h
   kcalc_statusbar.h
   kcalc_input_display.h
   kcalc_display_frame.h
   kcalc_parser.h
   kcalc_token.h
   kcalc_priority_levels_p.h
   kcalc_display.h
   kcalc_history.h
   kcalc_numeralsystem_view.h
   kcalc.qrc
)

ki18n_wrap_ui(kcalc
   kcalc.ui
   constants.ui
   colors.ui
   fonts.ui
   general.ui
)

ecm_qt_declare_logging_category(kcalc
    HEADER kcalc_debug.h
    IDENTIFIER KCALC_LOG
    CATEGORY_NAME org.kde.kcalc
    DESCRIPTION "kcalc"
)

kconfig_add_kcfg_files(kcalc kcalc_settings.kcfgc )


target_link_libraries(kcalc
    Qt6::Core
    Qt6::Widgets
    KF6::ColorScheme
    KF6::CoreAddons
    KF6::ConfigWidgets
    KF6::GuiAddons
    KF6::I18n
    KF6::Notifications
    KF6::XmlGui
    KF6::Crash
    knumber
)

if(BUILD_TESTING)
    add_library(kcalcparsercore STATIC
        kcalc_debug.cpp
        kcalc_parser.cpp
        kcalc_core.cpp
        kcalc_token.cpp
        kcalc_core_p.cpp
        kcalc_stats.cpp

        kcalc_debug.h
        kcalc_parser.h
        kcalc_core.h
        kcalc_token.h
        kcalc_core_p.h
        kcalc_stats.h
    )

    target_link_libraries(
        kcalcparsercore PUBLIC
        Qt6::Core
        Qt6::Xml
        knumber
    )
endif()

set_target_properties(kcalc PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist.in)

install( PROGRAMS org.kde.kcalc.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install( FILES kcalc.kcfg  DESTINATION ${KDE_INSTALL_KCFGDIR})
install( FILES org.kde.kcalc.appdata.xml   DESTINATION ${KDE_INSTALL_METAINFODIR})
