add_executable(TestSuite)


# Directories
set(SourceDir "src/")
set(IncludeDir "include/")


# Files
set(TestFiles "SampleTest.cpp")
list(TRANSFORM TestFiles PREPEND ${SourceDir})

set(IncludeFiles "Utils.h")
list(TRANSFORM IncludeFiles PREPEND ${IncludeDir})


target_sources(
    TestSuite
    PRIVATE
    ${TestFiles}

    PRIVATE
    FILE_SET HEADERS
    BASE_DIRS ${IncludeDir}
    FILES ${IncludeFiles}
)

target_include_directories(
    TestSuite PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(
    TestSuite PRIVATE
    GTest::gtest_main
    StrictWarnings
)

include(GoogleTest)
gtest_discover_tests(TestSuite)