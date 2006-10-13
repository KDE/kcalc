if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_SOURCE_DIR}/kcalc/knumber  )

set(knumbertest_SRCS knumbertest.cpp ${libknumber_la_SRCS})

kde4_automoc(knumbertest ${knumbertest_SRCS})

kde4_add_executable(knumbertest ${knumbertest_SRCS})

target_link_libraries(knumbertest  ${KDE4_KDECORE_LIBS} ${GMP_LIBRARIES} )

endif(KDE4_BUILD_TESTS)

