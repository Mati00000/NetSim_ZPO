cmake_minimum_required(VERSION 3.24)
set(PROJECT_ID netsim_project)
project(${PROJECT_ID})

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Werror -Wpedantic -pedantic-errors -Wconversion)

include_directories(
        include
        mocks
)

set(SOURCE_FILES
        src/simulation.cpp
        src/reports.cpp
        src/factory.cpp
        src/helpers.cpp
        src/nodes.cpp
        src/package.cpp
        src/storage_types.cpp
        )

add_executable(${PROJECT_ID} ${SOURCE_FILES} main.cpp)

set(SOURCE_FILES_TESTS
        test/test_factory_io.cpp
        test/test_Factory.cpp
        test/test_nodes.cpp
        test/test_package.cpp
        test/test_storage_types.cpp
        )

set(EXEC_TEST ${PROJECT_ID}_test)
add_executable(${EXEC_TEST} ${SOURCE_FILES} ${SOURCE_FILES_TESTS} test/main_gtest.cpp)
set(GTEST_ROOT ../googletest-master)
target_include_directories(${EXEC_TEST} PUBLIC
        ${GTEST_ROOT}/googlemock/include
        ${GTEST_ROOT}/googletest/include
        )

add_subdirectory( ${GTEST_ROOT} googletest-master)

target_link_libraries(${EXEC_TEST} gmock)