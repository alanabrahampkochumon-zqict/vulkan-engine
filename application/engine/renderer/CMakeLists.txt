add_library(
        TempestRenderer
        OBJECT
        ${vulkanhpp_SOURCE_DIR}

        Buffer.cpp
        CommandBuffer.cpp
        RenderPipeline.cpp
        RenderDevice.cpp
        GraphicsContext.cpp
        Swapchain.cpp
        PresentMode.cpp

        Buffer.h
        CommandBuffer.h
        RenderPipeline.h
        RenderDevice.h
        RenderQueue.h
        GraphicsContext.h
        Swapchain.h
        PresentMode.h
)

target_compile_features(TempestRenderer PRIVATE cxx_std_20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_link_libraries(
        TempestRenderer PRIVATE
        TempestUtils

        Vulkan::Headers
)