add_executable(
        GameEngine

        # src/OldVulkan.cpp
        Main.cpp
)
target_compile_features(GameEngine PUBLIC cxx_std_23)

target_sources(GameEngine PRIVATE
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        src/FileReader.h
)

target_include_directories(GameEngine PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_subdirectory(engine)

target_link_libraries(
        GameEngine
        PRIVATE
        StrictWarnings
        SDL3::SDL3-static
        #        ${Vulkan_LIBRARIES}

        TempestEngine
)

set(SHADER_SOURCES basic.slang)
AddSlangShaderTarget(GameEngine SOURCES ${SHADER_SOURCES})

add_custom_command(
        TARGET GameEngine
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders"
        "$<TARGET_FILE_DIR:GameEngine>/shaders"
        COMMENT "Copying shaders to build directory..."
)