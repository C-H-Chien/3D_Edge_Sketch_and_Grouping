SET(TEST_SOURCES test_functions)

FOREACH (test_src ${TEST_SOURCES})
    ADD_EXECUTABLE(${test_src} ${test_src}.cpp)
    TARGET_LINK_LIBRARIES(${test_src} edge_reconstruction)
    ADD_TEST(${test_src} ${test_src})
ENDFOREACH (test_src)