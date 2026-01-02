# Project requires a minimum version of CMake 3.20 to build
cmake_minimum_required(VERSION 3.20)

# Declaring project and enable C++ & Windows resource compilation
project(Basic_Win32_Application LANGUAGES CXX RC)

# Defining a GUI Windows executable target
add_executable(Basic_Win32_Application WIN32)

# Declaring source and resource files
target_sources(Basic_Win32_Application PRIVATE
        src/main.cpp
        src/ButtonManager.cpp
        src/WindowProcHandler.cpp

        resources/Resources.rc
        resources/Resource.h

        include/app/AppState.h
        include/app/ButtonManager.h
        include/app/WindowProcHandler.h
)

# Request C++ 23 and disable compiler extensions
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

# Local include paths
target_include_directories(Basic_Win32_Application PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
)

# Linking the required Windows library for DWM (dark mode)
target_link_libraries(Basic_Win32_Application PRIVATE dwmapi)

# Warnings (MSVC vs GCC/Clang)
if (MSVC)
    target_compile_options(Basic_Win32_Application PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(Basic_Win32_Application PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Fully static link for MinGW
if (MINGW)
    target_link_options(Basic_Win32_Application PRIVATE -static)
endif()