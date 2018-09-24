cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0057 NEW)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
include(GoogleTest)

add_executable(unit_tests test.cpp)
target_link_libraries(unit_tests memory_view GTest::GTest Threads::Threads)
target_compile_features(unit_tests PRIVATE cxx_std_17)
gtest_add_tests(TARGET unit_tests)
