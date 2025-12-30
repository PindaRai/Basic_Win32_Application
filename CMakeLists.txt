# Project requires a minimum version of CMake 3.20 to build
cmake_minimum_required(VERSION 3.20)
project(Basic_Win32_Application LANGUAGES CXX RC)

add_executable(Basic_Win32_Application WIN32
        AppState.h
        Resource.h)

target_sources(Basic_Win32_Application PRIVATE
        main.cpp
        ButtonManager.cpp
        WindowProcHandler.cpp
        Resources.rc
)

# Request C++23 and disables compiler extensions
target_compile_features(Basic_Win32_Application PRIVATE cxx_std_23)
set_target_properties(Basic_Win32_Application PROPERTIES
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

# Preprocessor definitions (Unicode + avoid Windows macro conflicts)
target_compile_definitions(Basic_Win32_Application PRIVATE
        UNICODE _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
)

# Local include path
target_include_directories(Basic_Win32_Application PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link required Windows library
target_link_libraries(Basic_Win32_Application PRIVATE dwmapi)

# Warnings (MSVC vs GCC/Clang)
if (MSVC)
    target_compile_options(Basic_Win32_Application PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(Basic_Win32_Application PRIVATE -Wall -Wextra -Wpedantic)
endif()

# -static only applies to MinGW/GCC toolchains
if (MINGW)
    target_link_options(Basic_Win32_Application PRIVATE -static)
endif()