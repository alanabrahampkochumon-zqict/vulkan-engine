add_library(
        TempestPlatform
        OBJECT
        ${vulkanhpp_SOURCE_DIR}
        Window.cpp
        TempestSurface.cpp

        Window.h
        TempestSurface.h
)

target_compile_features(TempestPlatform PRIVATE cxx_std_20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_link_libraries(
        TempestPlatform PRIVATE
        SDL3::SDL3-static
        Vulkan::Headers
)