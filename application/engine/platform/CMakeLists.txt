add_library(
        TempestPlatform
        OBJECT
        ${vulkanhpp_SOURCE_DIR}
        TempestWindow.cpp

        TempestWindow.h
)

target_compile_features(TempestPlatform PRIVATE cxx_std_20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_link_libraries(
        TempestPlatform PUBLIC # TODO: Update to private after init vulkan using pimpl idiom
        SDL3::SDL3-static
        Vulkan::Headers
)