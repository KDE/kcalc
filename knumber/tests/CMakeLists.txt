set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/knumber  )

set(knumbertest_SRCS knumbertest.cpp ${libknumber_la_SRCS})

kde4_add_unit_test(knumbertest TESTNAME KNumber ${knumbertest_SRCS})

target_link_libraries(knumbertest ${KDE4_KDECORE_LIBS} ${GMP_LIBRARIES})
