set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/knumber  )

set(knumbertest_SRCS knumbertest.cpp ${libknumber_la_SRCS})

add_executable(knumbertest ${knumbertest_SRCS})
add_test(KNumber knumbertest)
ecm_mark_as_test(knumbertest)

target_link_libraries(knumbertest Qt5::Core ${GMP_LIBRARIES})
