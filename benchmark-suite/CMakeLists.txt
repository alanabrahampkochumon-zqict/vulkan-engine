add_executable(BenchmarkSuite)


#Directories
set(SourceDir "src/")


# Files
set(SrcFiles "SampleBenchmark.cpp")
list(TRANSFORM SrcFiles PREPEND ${SourceDir})


target_sources(
    BenchmarkSuite
    PRIVATE
    ${SrcFiles}
)

target_precompile_headers(
    BenchmarkSuite
    PRIVATE
    <benchmark/benchmark.h>
)

target_include_directories(BenchmarkSuite PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(
    BenchmarkSuite
    PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
    StrictWarnings
)