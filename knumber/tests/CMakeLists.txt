#    SPDX-FileCopyrightText: 2023 Gabriel Barrantes <gabriel.barrantes.dev@outlook.com>

#    SPDX-License-Identifier: GPL-2.0-or-later

add_definitions( -DKDESRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/" )
include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

find_package(Qt${QT_MAJOR_VERSION}Test CONFIG REQUIRED)
include(ECMAddTests)

ecm_add_test(knumber_aritmetic_test.cpp
    LINK_LIBRARIES knumber
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Test
    TEST_NAME knumber_aritmetic_test
)

ecm_add_test(knumber_bitwise_operators_test.cpp
    LINK_LIBRARIES knumber
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Test
    TEST_NAME knumber_bitwise_operators_test
)

ecm_add_test(knumber_constans_test.cpp
    LINK_LIBRARIES knumber
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Test
    TEST_NAME knumber_constans_test
)

ecm_add_test(knumber_integer_operators_test.cpp
    LINK_LIBRARIES knumber
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Test
    TEST_NAME knumber_integer_operators_test
)

ecm_add_test(knumber_functions_test.cpp
    LINK_LIBRARIES knumber
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Test
    TEST_NAME knumber_functions_test
)

ecm_add_test(knumber_misc_test.cpp
    LINK_LIBRARIES knumber
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Test
    TEST_NAME knumber_misc_test
)
