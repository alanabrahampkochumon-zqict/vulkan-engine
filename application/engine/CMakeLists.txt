add_library(TempestEngine OBJECT)

target_compile_features(TempestEngine PUBLIC cxx_std_23)

target_sources(
        TempestEngine
        PRIVATE
        TempestEngine.cpp
        GEWindow.cpp
        ${vulkanhpp_SOURCE_DIR}

        PUBLIC
        FILE_SET CXX_MODULES
        FILES
        TempestEngine.cppm
        GEWindow.cppm

        PUBLIC
        FILE_SET HEADERS
        FILES
        FileReader.h
)


target_link_libraries(
        TempestEngine PRIVATE

        # StrictWarnings
        SDL3::SDL3-static
        Vulkan::Headers
)