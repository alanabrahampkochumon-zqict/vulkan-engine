add_library(TempestEngine STATIC)

target_compile_features(TempestEngine PUBLIC cxx_std_23)

add_subdirectory(platform)
add_subdirectory(renderer)
add_subdirectory(utils)

target_sources(
        TempestEngine
        PRIVATE
        TempestEngine.cpp
        Application.cpp

        ${vulkanhpp_SOURCE_DIR}

        PUBLIC
        FILE_SET CXX_MODULES
        FILES
        TempestEngine.cppm
        Vertex.cppm
        UniformBufferObject.cppm

        PUBLIC
        FILE_SET HEADERS
        FILES
        FileReader.h
        Application.h
)


target_link_libraries(
        TempestEngine PRIVATE

        # Object Linkage
        TempestPlatform
        TempestRenderer
        TempestUtils

        # StrictWarnings
        SDL3::SDL3-static # TODO: REMOVE
        Vulkan::Headers # TODO: REMOVE
        glm::glm # TODO: Replace with Falcon
        std::stb
        tinyobjloader
)