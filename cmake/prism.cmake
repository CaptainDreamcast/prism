set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Add source/header files to project
file(GLOB_RECURSE PRISM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/../*.cpp SOURCES ../*.h)
list(FILTER PRISM_SOURCES EXCLUDE REGEX ".*/dc/.*")
list(FILTER PRISM_SOURCES EXCLUDE REGEX ".*/vita/.*")
list(FILTER PRISM_SOURCES EXCLUDE REGEX ".*/web/.*")
list(FILTER PRISM_SOURCES EXCLUDE REGEX ".*/.git.*")
list(FILTER PRISM_SOURCES EXCLUDE REGEX ".*/assets/.*")
list(FILTER PRISM_SOURCES EXCLUDE REGEX ".*/other/.*")
list(FILTER PRISM_SOURCES EXCLUDE REGEX ".*/tools/.*")

# Define Dream
add_library(prism STATIC ${PRISM_SOURCES})

# Add include paths
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/SDL2-2.0.7/include)
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/SDL2_image-2.0.1)
target_include_directories(prism PUBLIC C:/Program Files (x86)/FMOD SoundSystem)
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/SDL2_ttf-2.0.14)
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/lpng1632)
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/glew-2.1.0/include)
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/zstd/lib)
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/zlib-1.2.11)
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/optick/src)
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/openai-cpp/include)
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/curl-7.88.1/include)
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/gpt4all/gpt4all-backend)
target_include_directories(prism PUBLIC C:/DEV/PLATFORMS/WINDOWS/LIBS/enet-1.3.17/include)
target_include_directories(prism PUBLIC C:/DEV/PROJECTS/addons/prism/external/imgui/inc)
target_include_directories(prism PUBLIC C:/DEV/PROJECTS/addons/prism/external/imgui_texteditor/inc)
target_include_directories(prism PUBLIC C:/DEV/PROJECTS/addons/prism/include)

target_compile_definitions(prism PUBLIC UNICODE)
target_compile_definitions(prism PUBLIC _UNICODE)
target_compile_definitions(prism PUBLIC _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(prism PUBLIC GLEW_STATIC)

set_property(TARGET prism PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_compile_options(prism PRIVATE /Gy)

# Add libraries
add_library(SDL2 STATIC IMPORTED)
set_target_properties(SDL2 PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/SDL2.lib"
  INTERFACE_INCLUDE_DIRECTORIES "C:/DEV/PLATFORMS/WINDOWS/LIBS/SDL2-2.0.7/include"
)

add_library(SDL2main STATIC IMPORTED)
set_target_properties(SDL2main PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/SDL2main.lib"
)

add_library(SDL2_image STATIC IMPORTED)
set_target_properties(SDL2_image PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/SDL2_image.lib"
  INTERFACE_INCLUDE_DIRECTORIES "C:/DEV/PLATFORMS/WINDOWS/LIBS/SDL2_image-2.0.1"
)

add_library(fmod STATIC IMPORTED)
set_target_properties(fmod PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/fmod_vc.lib"
  INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files (x86)/FMOD SoundSystem"
)

add_library(SDL2_ttf STATIC IMPORTED)
set_target_properties(SDL2_ttf PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/SDL2_ttf.lib"
  INTERFACE_INCLUDE_DIRECTORIES "C:/DEV/PLATFORMS/WINDOWS/LIBS/SDL2_ttf-2.0.14"
)

add_library(enet STATIC IMPORTED)
set_target_properties(enet PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/enet.lib"
)

add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/freetype.lib"
)

add_library(glew STATIC IMPORTED)
set_target_properties(glew PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/glew32s.lib"
  INTERFACE_INCLUDE_DIRECTORIES "C:/DEV/PLATFORMS/WINDOWS/LIBS/glew-2.1.0/include"
)

add_library(curl STATIC IMPORTED)
set_target_properties(curl PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/libcurl.lib"
  INTERFACE_INCLUDE_DIRECTORIES "C:/DEV/PLATFORMS/WINDOWS/LIBS/curl-7.88.1/include"
)

add_library(libjpeg STATIC IMPORTED)
set_target_properties(libjpeg PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/libjpeg.lib"
)

add_library(libpng STATIC IMPORTED)
set_target_properties(libpng PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/libpng16.lib"
  INTERFACE_INCLUDE_DIRECTORIES "C:/DEV/PLATFORMS/WINDOWS/LIBS/lpng1632"
)

add_library(libwebp STATIC IMPORTED)
set_target_properties(libwebp PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/libwebp.lib"
)

add_library(zstd STATIC IMPORTED)
set_target_properties(zstd PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/libzstd_static.lib"
  INTERFACE_INCLUDE_DIRECTORIES "C:/DEV/PLATFORMS/WINDOWS/LIBS/zstd/lib"
)

add_library(zlib STATIC IMPORTED)
set_target_properties(zlib PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../windows/vs17/LIB/zlib.lib"
  INTERFACE_INCLUDE_DIRECTORIES "C:/DEV/PLATFORMS/WINDOWS/LIBS/zlib-1.2.11"
)

add_library(crypt32 STATIC IMPORTED)
set_target_properties(crypt32 PROPERTIES
  IMPORTED_LOCATION "crypt32.lib"
)

add_library(wldap32 STATIC IMPORTED)
set_target_properties(wldap32 PROPERTIES
  IMPORTED_LOCATION "wldap32.lib"
)

add_library(ws2_32 STATIC IMPORTED)
set_target_properties(ws2_32 PROPERTIES
  IMPORTED_LOCATION "ws2_32.lib"
)

add_library(winmm STATIC IMPORTED)
set_target_properties(winmm PROPERTIES
  IMPORTED_LOCATION "winmm.lib"
)

add_library(OpenGL32 STATIC IMPORTED)
set_target_properties(OpenGL32 PROPERTIES
  IMPORTED_LOCATION "OpenGL32.lib"
)

add_library(version STATIC IMPORTED)
set_target_properties(version PROPERTIES
  IMPORTED_LOCATION "version.lib"
)