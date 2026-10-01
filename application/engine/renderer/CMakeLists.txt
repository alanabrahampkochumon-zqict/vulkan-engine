add_library(
        TempestRenderer
        OBJECT
        ${vulkanhpp_SOURCE_DIR}

        Buffer.cpp
        CommandBuffer.cpp
        RenderPipeline.cpp
        RenderDevice.cpp
        Image.cpp
        GraphicsContext.cpp
        Swapchain.cpp
        PresentMode.cpp
        TempestRenderer.cpp
        Texture.cpp

        Buffer.h
        CommandBuffer.h
        RenderPipeline.h
        RenderDevice.h
        RenderQueue.h
        PipelineConfig.h
        Image.h
        GraphicsContext.h
        Swapchain.h
        PresentMode.h
        TempestRenderer.h
        Texture.h
        Vertex.h
)

target_compile_features(TempestRenderer PRIVATE cxx_std_20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_link_libraries(
        TempestRenderer PUBLIC # TODO: Update to private after init vulkan using pimpl idiom
        TempestUtils
        std::stb
        Vulkan::Headers
        glm::glm
)