include(ECMAddTests)

ecm_add_test(knumbertest.cpp
    LINK_LIBRARIES knumber
    TEST_NAME knumbertest
)
