cmake_minimum_required(VERSION 3.25) 
project(Template CXX)               
set(CMAKE_CXX_STANDARD 17)            
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include("${CMAKE_SOURCE_DIR}/../../addons/prism/cmake/prism.cmake" REQUIRED)

# Add source/header files to project
file(GLOB_RECURSE SOURCES ../*.cpp SOURCES ../*.h SOURCES ../*.rc)
list(FILTER SOURCES EXCLUDE REGEX ".*web.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/build/.*")

add_link_options(/NODEFAULTLIB:libcmt.lib)
add_link_options(/IGNORE:4099,4286,4098)
add_link_options(/OPT:REF)
add_link_options(/OPT:ICF)

# Define exe
add_executable(Template ${SOURCES})

# Link with libraries
target_link_libraries(Template prism)
target_link_libraries(Template SDL2)
target_link_libraries(Template SDL2main)
target_link_libraries(Template SDL2_image)
target_link_libraries(Template fmod)
target_link_libraries(Template SDL2_ttf)
target_link_libraries(Template enet)
target_link_libraries(Template freetype)
target_link_libraries(Template glew)
target_link_libraries(Template curl)
target_link_libraries(Template libjpeg)
target_link_libraries(Template libpng)
target_link_libraries(Template libwebp)
target_link_libraries(Template zstd)
target_link_libraries(Template zlib)
target_link_libraries(Template crypt32)
target_link_libraries(Template wldap32)
target_link_libraries(Template ws2_32)
target_link_libraries(Template winmm)
target_link_libraries(Template OpenGL32)
target_link_libraries(Template version)

# Define preprocessor definitions
target_compile_definitions(Template PUBLIC UNICODE)
target_compile_definitions(Template PUBLIC _UNICODE)

set_property(TARGET Template PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_compile_options(Template PRIVATE /Gy)

set_target_properties(Template PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../assets)
set_target_properties(Template PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/../assets)

# Copy over DLLs
file(GLOB DLLS "${CMAKE_SOURCE_DIR}/../../addons/prism/windows/vs17/DLL/*.dll")
foreach(DLL ${DLLS})
  add_custom_command(TARGET Template POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DLL}
    $<TARGET_FILE_DIR:Template>)
endforeach()

# Optimize for release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(Template PRIVATE /O2)
endif()