add_library(
        TempestRenderer
        OBJECT
        ${vulkanhpp_SOURCE_DIR}

        RenderPipeline.cpp
        RenderDevice.cpp
        GraphicsContext.cpp

        RenderPipeline.h
        RenderDevice.h
        RenderQueue.h
        GraphicsContext.h
)

target_compile_features(TempestRenderer PRIVATE cxx_std_20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_link_libraries(
        TempestRenderer PRIVATE
        SDL3::SDL3-static
        Vulkan::Headers
)