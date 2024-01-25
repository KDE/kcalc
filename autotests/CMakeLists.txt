#    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>

#    SPDX-License-Identifier: GPL-2.0-or-later

add_definitions( -DKDESRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/" )
include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

find_package(Qt${QT_MAJOR_VERSION}Test CONFIG REQUIRED)
include(ECMAddTests)

ecm_add_test(kcalc_parser_core_test.cpp ../kcalc_parser.cpp ../kcalc_core.cpp ../kcalc_token.cpp ../kcalc_core_p.cpp ../stats.cpp
TEST_NAME "kcalc_parser_core_test"
LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Core
Qt${QT_MAJOR_VERSION}::Test
Qt${QT_MAJOR_VERSION}::Xml
knumber
)



