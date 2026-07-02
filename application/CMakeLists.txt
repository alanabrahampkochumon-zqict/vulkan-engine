add_executable(VulkanEngine)
target_compile_features(VulkanEngine PUBLIC cxx_std_20)

#Directories
set(SourceDir "src/")


# Files
set(SrcFiles "Main.cpp")
list(TRANSFORM SrcFiles PREPEND ${SourceDir})


target_sources(
        VulkanEngine
        PRIVATE
        ${SrcFiles}
)

target_include_directories(VulkanEngine PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)


target_link_libraries(
        VulkanEngine
        PRIVATE
        StrictWarnings
        SDL3::SDL3-static
        ${Vulkan_LIBRARIES}
)