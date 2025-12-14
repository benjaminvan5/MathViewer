cmake_minimum_required(VERSION 3.20)
project(PDFEngine C)

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Source files
file(GLOB SOURCES "src/*.c")

# Add executable
add_executable(pdfengine ${SOURCES})

# Optional: Enable warnings
if(MSVC)
    target_compile_options(pdfengine PRIVATE /W4 /WX)
else()
    target_compile_options(pdfengine PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(pdfengine ${SDL2_LIBRARIES})

# Find FreeType
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
target_link_libraries(pdfengine ${FREETYPE_LIBRARIES})
