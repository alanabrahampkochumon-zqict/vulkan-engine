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

# https://stackoverflow.com/questions/69144529/vulkan-hpp-with-cmake
# Declare required Vulkan version
set(VULKAN_VER_MAJOR 1)
set(VULKAN_VER_MINOR 4)
set(VULKAN_VER_PATCH 358)
set(VULKAN_VERSION ${VULKAN_VER_MAJOR}.${VULKAN_VER_MINOR}.${VULKAN_VER_PATCH})

#-------------------------------------------------------------------------
# Fetch Vulkan C Headers
# ------------------------------------------------------------------------
message(STATUS "Include Vulkan C Headers")
FetchContent_Declare(
        VulkanHeaders
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
        GIT_TAG v${VULKAN_VERSION}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
set(PROJECT_IS_TOP_LEVEL OFF)
FetchContent_MakeAvailable(VulkanHeaders) # vulkanheaders has a proper CMakeLists

#-------------------------------------------------------------------------
# Fetch Vulkan C++ Headers
#-------------------------------------------------------------------------
message(STATUS "Include Vulkan C++ Headers")
FetchContent_Declare(
        VulkanHPP
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Hpp.git
        GIT_TAG v${VULKAN_VERSION}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
# vulkanhpp does not have a proper CMakeLists
# FetchContent_GetProperties(VulkanHPP)
# FetchContent_Populate(VulkanHPP)
FetchContent_MakeAvailable(VulkanHPP)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE) # Disables google benchmark from creating its own test suite

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(googlebenchmark)
FetchContent_MakeAvailable(sdl3)

# Group the projects into a single folder
set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER "Google Test")
set_target_properties(benchmark benchmark_main PROPERTIES FOLDER "Google Benchmark")