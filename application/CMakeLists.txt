add_executable(
        VulkanEngine

        src/Main.cpp
        src/FileReader.h
)
target_compile_features(VulkanEngine PUBLIC cxx_std_23)

target_sources(VulkanEngine PRIVATE
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        src/FileReader.h
)

target_include_directories(VulkanEngine PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_subdirectory(renderer)

target_link_libraries(
        VulkanEngine
        PRIVATE
        StrictWarnings
        SDL3::SDL3-static
        ${Vulkan_LIBRARIES}

        VulkanRenderer
)


add_custom_command(
        TARGET VulkanEngine
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders"
        "$<TARGET_FILE_DIR:VulkanEngine>/shaders"
        COMMENT "Copying shaders to build directory"
)