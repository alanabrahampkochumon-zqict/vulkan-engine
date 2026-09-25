add_library(
        TempestUtils
        OBJECT
        Logger.cpp

        FileReader.h
        Logger.h
)

target_compile_features(TempestUtils PRIVATE cxx_std_20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_link_libraries(
        TempestUtils PRIVATE
        quill::quill
)