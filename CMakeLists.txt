cmake_minimum_required(VERSION 3.16)
project(DoomLike VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g -O0")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Zi /Od")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find packages
find_package(OpenGL REQUIRED)

# Platform-specific library paths
if(WIN32)
    # MSYS2/MinGW paths
    list(APPEND CMAKE_PREFIX_PATH "C:/msys64/mingw64")
    list(APPEND CMAKE_LIBRARY_PATH "C:/msys64/mingw64/lib")
    list(APPEND CMAKE_INCLUDE_PATH "C:/msys64/mingw64/include")
endif()

# GLEW
find_package(GLEW QUIET)
if(NOT GLEW_FOUND)
    find_path(GLEW_INCLUDE_DIR GL/glew.h 
        PATHS 
            /usr/include 
            /usr/local/include
            C:/msys64/mingw64/include
    )
    find_library(GLEW_LIBRARY NAMES GLEW glew32 glew32.dll 
        PATHS 
            /usr/lib 
            /usr/local/lib
            C:/msys64/mingw64/lib
    )
    if(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
        set(GLEW_FOUND TRUE)
        set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
        set(GLEW_LIBRARIES ${GLEW_LIBRARY})
        message(STATUS "Found GLEW: ${GLEW_LIBRARIES}")
    endif()
endif()

# GLUT (freeglut on MSYS2)
find_package(GLUT QUIET)
if(NOT GLUT_FOUND)
    find_path(GLUT_INCLUDE_DIR GL/glut.h GL/freeglut.h 
        PATHS 
            /usr/include 
            /usr/local/include
            C:/msys64/mingw64/include
    )
    find_library(GLUT_LIBRARY NAMES glut freeglut freeglut.dll glut32 
        PATHS 
            /usr/lib 
            /usr/local/lib
            C:/msys64/mingw64/lib
    )
    if(GLUT_INCLUDE_DIR AND GLUT_LIBRARY)
        set(GLUT_FOUND TRUE)
        set(GLUT_INCLUDE_DIRS ${GLUT_INCLUDE_DIR})
        set(GLUT_LIBRARIES ${GLUT_LIBRARY})
        message(STATUS "Found GLUT: ${GLUT_LIBRARIES}")
    endif()
endif()

# OpenAL
find_package(OpenAL QUIET)
if(NOT OpenAL_FOUND)
    find_path(OPENAL_INCLUDE_DIR AL/al.h 
        PATHS 
            /usr/include 
            /usr/local/include
            C:/msys64/mingw64/include
            C:/Program Files/OpenAL 1.1 SDK/include
    )
    find_library(OPENAL_LIBRARY NAMES openal OpenAL32 openal.dll libopenal.dll.a 
        PATHS 
            /usr/lib 
            /usr/local/lib
            C:/msys64/mingw64/lib
            "C:/Program Files/OpenAL 1.1 SDK/libs/Win64"
    )
    if(OPENAL_INCLUDE_DIR AND OPENAL_LIBRARY)
        set(OpenAL_FOUND TRUE)
        set(OPENAL_INCLUDE_DIR ${OPENAL_INCLUDE_DIR})
        set(OPENAL_LIBRARY ${OPENAL_LIBRARY})
        message(STATUS "Found OpenAL: ${OPENAL_LIBRARY}")
    endif()
endif()

# Check required libraries
if(NOT GLEW_FOUND)
    message(FATAL_ERROR "GLEW not found!")
endif()
if(NOT GLUT_FOUND)
    message(FATAL_ERROR "GLUT not found!")
endif()
if(NOT OpenAL_FOUND)
    message(FATAL_ERROR "OpenAL not found!")
endif()

# Include directories for found libraries
include_directories(
    ${GLEW_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
)

# Gather all source files
file(GLOB_RECURSE SOURCES 
    ${CMAKE_SOURCE_DIR}/src/**/*.cpp
)

# Add main.cpp
list(APPEND SOURCES ${CMAKE_SOURCE_DIR}/main.cpp)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    OpenGL::GL
    OpenGL::GLU
    ${GLEW_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${OPENAL_LIBRARY}
)

# Windows-specific: copy DLLs to output directory
if(WIN32)
    # Find and copy GLEW DLL
    if(GLEW_LIBRARY)
        get_filename_component(GLEW_DIR ${GLEW_LIBRARY} DIRECTORY)
        file(GLOB GLEW_DLL "${GLEW_DIR}/glew32.dll")
        if(GLEW_DLL)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${GLEW_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
        endif()
    endif()
    
    # Find and copy GLUT DLL
    if(GLUT_LIBRARY)
        get_filename_component(GLUT_DIR ${GLUT_LIBRARY} DIRECTORY)
        file(GLOB GLUT_DLL "${GLUT_DIR}/freeglut.dll" "${GLUT_DIR}/glut32.dll")
        if(GLUT_DLL)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${GLUT_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
        endif()
    endif()
    
    # Find and copy OpenAL DLL
    if(OPENAL_LIBRARY)
        get_filename_component(OPENAL_DIR ${OPENAL_LIBRARY} DIRECTORY)
        file(GLOB OPENAL_DLL "${OPENAL_DIR}/OpenAL32.dll" "${OPENAL_DIR}/openal.dll")
        if(OPENAL_DLL)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${OPENAL_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
        endif()
    endif()
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Copy assets, maps, and shaders to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/maps $<TARGET_FILE_DIR:${PROJECT_NAME}>/maps
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMENT "Copying assets, maps, and shaders to build directory"
)
