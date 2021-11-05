add_library(knumber STATIC
    knumber.cpp
    knumber_error.cpp
    knumber_float.cpp
    knumber_fraction.cpp
    knumber_integer.cpp
    knumber_operators.cpp
    knumber.h
    knumber_fraction.h
    knumber_operators.h
    knumber_base.h
    knumber_error.h
    knumber_integer.h
    knumber_float.h

)

target_include_directories(knumber PUBLIC ${CMAKE_BINARY_DIR} ${GMP_INCLUDE_DIR})

target_link_libraries(knumber PUBLIC Qt::Core PRIVATE ${GMP_LIBRARIES} ${MPFR_LIBRARIES})

add_subdirectory( tests ) 
