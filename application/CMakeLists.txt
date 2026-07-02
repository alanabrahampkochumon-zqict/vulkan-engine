add_executable(VulkanEngine)


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
)