include(FetchContent)

set(SDL_SHARED OFF CACHE BOOL "Build shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build static library" FORCE)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723 # release-1.17.0
    SYSTEM
)

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.5
    SYSTEM
)

FetchContent_Declare(
    sdl3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG bd7c9e467bed6ec5103925fc1fb0a9e3703d6e54
    SYSTEM
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE) # Disables google benchmark from creating its own test suite

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(googlebenchmark)
FetchContent_MakeAvailable(sdl3)

# Group the projects into a single folder
set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER "Google Test")
set_target_properties(benchmark benchmark_main PROPERTIES FOLDER "Google Benchmark")