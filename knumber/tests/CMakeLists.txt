include_directories( ${CMAKE_SOURCE_DIR}/knumber  )

include(ECMAddTests)

ecm_add_test(knumbertest.cpp ${libknumber_la_SRCS}
    LINK_LIBRARIES Qt5::Core ${GMP_LIBRARIES}
    TEST_NAME knumbertest
)

