include_guard()
# TODO: Fix this
function(AddSlangShaderTarget Target)
    cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
    set(SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/res/shaders)
    set(ENTRY_POINTS -entry vertMain -entry fragMain)

    # Make a directory
    add_custom_command(
            OUTPUT ${SHADERS_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
    )
    # Run the slang compiler
    add_custom_command(
            OUTPUT ${SHADERS_DIR}/basic.spv
            COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCE} -target spirv -profile spirv_1_4 -emit-spirv-directly -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o basic.spv
            WORKING_DIRECTORY ${SHADERS_DIR}
            DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
            COMMENT "Compiling Slang Shaders"
            VERBATIM
    )
    add_custom_target(${TARGET} DEPENDS ${SHADERS_DIR}/basic.spv)
endfunction()