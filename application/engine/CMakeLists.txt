add_library(VulkanEngine OBJECT)

target_compile_features(VulkanEngine PUBLIC cxx_std_23)

target_sources(
        VulkanEngine
        PRIVATE
        VulkanEngine.cpp

        PUBLIC
        FILE_SET CXX_MODULES
        FILES
        VulkanEngine.cppm
)

target_link_libraries(
        VulkanEngine PRIVATE

        # StrictWarnings
        SDL3::SDL3-static
)