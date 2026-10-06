cmake_minimum_required(VERSION 3.30)
project(Tempest LANGUAGES CXX)
set(LIBRARY_NAME Tempest)
enable_testing()


# Vulkan
#find_package(Vulkan REQUIRED)
#if (NOT Vulkan_FOUND)
#    message(WARNING "Vulkan SDK not found! Make sure you have vulkan installed and put in the system path")
#endif ()
## Add include directories
include_directories(${Vulkan_INCLUDE_DIRS})


add_library(${PROJECT_NAME}-StrictWarnings INTERFACE) # Used for linking Warning flags
add_library(${PROJECT_NAME}-ASan INTERFACE) # Used for linking ASan

# Turn on solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(DevToolsDir "Dev Tools")

# Clang Formatter
find_program(CLANG_FORMATTER NAMES clang-format)

if (CLANG_FORMATTER)
    file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h *.cxx *.hxx *.hpp *.cc *.hh *.tpp *.inl)

    add_custom_target(${PROJECT_NAME}-Format
            COMMAND ${CLANG_FORMATTER} -i -style=file ${ALL_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Formatting source files..."
    )
    set_target_properties(${PROJECT_NAME}-Format PROPERTIES FOLDER ${DevToolsDir})
else ()
    message(WARNING "clang-format not found")
endif ()


# Clang Tidy
find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if (CLANG_TIDY_EXE)
    # Create a custom target that you manually trigger
    add_custom_target(
            ${PROJECT_NAME}-Tidy
            COMMAND ${CMAKE_COMMAND} -E echo "Running Clang-Tidy on MathLib..."
            COMMAND run-clang-tidy -p ${CMAKE_BINARY_DIR} -header-filter=.*
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running static analysis..."
    )
    set_target_properties(${PROJECT_NAME}-Tidy PROPERTIES FOLDER ${DevToolsDir})
else ()
    message(WARNING "clang-tidy not found")
endif ()


# Doxygen
find_package(Doxygen)
if (DOXYGEN_FOUND)
    message(STATUS "Doxygen found!")
    doxygen_add_docs(
            ${PROJECT_NAME}-Docs
            ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating documentation with doxygen..."
            CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
            ALL
    )
    set_target_properties(${PROJECT_NAME}-Docs PROPERTIES FOLDER ${DevToolsDir})
else ()
    message(WARN "Doxygen not found! Skipping documentation generation.")
endif ()


option(ENABLE_STRICT "Enable Strict Warnings" OFF) # Option to enable compiler specific "Warning as Error"
option(ASAN "Enable ASan" OFF) # Option to enable ASAN

# Enables Strict Warnings and Debug-only ASan
if (ENABLE_STRICT)
    include(cmake/StrictMode.cmake)
    AddStrictMode(${PROJECT_NAME}-StrictWarnings)
endif ()


# Enables Address Sanitizer for all the projects globally.
if (ASAN)
    message(STATUS "Turning on Address Sanitizer for Debug Builds")
    if (MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${PROJECT_NAME}-StrictWarnings INTERFACE $<$<CONFIG:Debug>:/fsanitize=address;/Zi>)
        target_link_options(${PROJECT_NAME}-StrictWarnings INTERFACE $<$<CONFIG:Debug>:/INCREMENTAL:NO>)
    else ()
        target_compile_options(${PROJECT_NAME}-StrictWarnings INTERFACE $<$<CONFIG:Debug>:-fsanitize=address;-fno-omit-frame-pointer>)
        target_link_options(${PROJECT_NAME}-StrictWarnings INTERFACE $<$<CONFIG:Debug>:-fsanitize=address>)
    endif ()
endif ()

# Multi-processor compilation
if (MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Enabling Multiprocessor compilation on MSVC")
    add_compile_options("/MP")
endif ()

include(dependencies.cmake)
include(cmake/SlangCompile.cmake)

add_subdirectory(test-suite)
add_subdirectory(application)


set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TestSuite)