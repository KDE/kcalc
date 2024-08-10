include(CheckTypeSize)
include(CheckIncludeFiles)

check_include_files(ieeefp.h     HAVE_IEEEFP_H)
check_type_size("signed long"    SIZEOF_SIGNED_LONG)
check_type_size("unsigned long"  SIZEOF_UNSIGNED_LONG)

configure_file(config-knumber.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-knumber.h )

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

find_package(MPC)
set_package_properties(MPC PROPERTIES
    DESCRIPTION "GNU MPC is a C library for the arithmetic of complex numbers with arbitrarily high precision and correct rounding of the result"
    URL "https://www.multiprecision.org/mpc/"
    TYPE REQUIRED
    PURPOSE "Required for building KCalc."
)

add_library(knumber STATIC
    knumber.cpp
    knumber_error.cpp
    knumber_complex.cpp
    knumber_float.cpp
    knumber_fraction.cpp
    knumber_integer.cpp
    knumber_operators.cpp
    knumber.h
    knumber_complex.h
    knumber_fraction.h
    knumber_operators.h
    knumber_base.h
    knumber_error.h
    knumber_integer.h
    knumber_float.h

)

target_include_directories(knumber PUBLIC ${CMAKE_BINARY_DIR} ${GMP_INCLUDE_DIR})

target_link_libraries(knumber PUBLIC Qt6::Core PRIVATE ${GMP_LIBRARIES} ${MPFR_LIBRARIES} ${MPC_LIBRARIES})

add_subdirectory( tests ) 
