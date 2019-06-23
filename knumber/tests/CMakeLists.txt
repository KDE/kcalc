include_directories( ${CMAKE_SOURCE_DIR}/knumber  )

include(ECMAddTests)

list(APPEND knumbertest_LIBS Qt5::Core ${MPFR_LIBRARIES} ${GMP_LIBRARIES})

ecm_add_test(knumbertest.cpp ${libknumber_la_SRCS}
    LINK_LIBRARIES ${knumbertest_LIBS}
    TEST_NAME knumbertest
)
