cmake_minimum_required(VERSION 3.10)
project(rest_client C)
find_package(PkgConfig REQUIRED)
include(FindPkgConfig)

# Ajustar sufijos para encontrar import libraries .dll.a en MSYS2 MinGW64
if(MINGW)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a" ".a" ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3 -pthread")

# Buscar cJSON manualmente en MSYS2 y sistemas Unix
find_path(CJSON_INCLUDE_DIR
    NAMES cJSON.h
    PATH_SUFFIXES cjson
    HINTS /mingw64/include ${CMAKE_INSTALL_PREFIX}/include /usr/local/include /usr/include
)
find_library(CJSON_LIBRARY
    NAMES cjson libcjson
    HINTS /mingw64/lib ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib /usr/lib
)
if(NOT CJSON_INCLUDE_DIR OR NOT CJSON_LIBRARY)
    message(FATAL_ERROR "cJSON no encontrado. Asegúrese de que cJSON.h y lib(c)json existan en /mingw64/include y /mingw64/lib. Instale mingw-w64-x86_64-cjson via pacman en MSYS2.")
endif()

## Usar pkg-config para libcurl (pkg_check_modules(CURL libcurl))
pkg_check_modules(CURL REQUIRED libcurl)

add_library(rest_client STATIC src/rest_client.c)
target_include_directories(rest_client PUBLIC
    ${CURL_INCLUDE_DIRS}
    ${CJSON_INCLUDE_DIR}
    src
)
target_link_libraries(rest_client PUBLIC
    ${CURL_LIBRARIES}
    ${CJSON_LIBRARY}
)

add_executable(example examples/example.c)
target_link_libraries(example PRIVATE rest_client ${CURL_LIBRARIES} ${CJSON_LIBRARY})

add_executable(obp_example examples/obp_example.c)
target_link_libraries(obp_example PRIVATE rest_client ${CURL_LIBRARIES} ${CJSON_LIBRARY})

add_executable(benchMark examples/benchMark.c)
target_link_libraries(benchMark PRIVATE rest_client ${CURL_LIBRARIES} ${CJSON_LIBRARY})
