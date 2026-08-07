add_library(VulkanRenderer OBJECT)

target_compile_features(VulkanRenderer PUBLIC cxx_std_23)

target_sources(
        VulkanRenderer
        PRIVATE
        VulkanRenderer.cpp

        PUBLIC
        FILE_SET CXX_MODULES
        FILES
        VulkanRenderer.cppm
)

target_link_libraries(
        VulkanRenderer PRIVATE

        StrictWarnings
        SDL3::SDL3-static
)