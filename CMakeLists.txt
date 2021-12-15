cmake_minimum_required(VERSION 3.17)
project(libonionshare C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

set(LIBONIONSHARE_BUILD_TYPE "static" CACHE STRING "The type of library to create: static, object, or shared (default static).")

# creates a resources header file from files in given directory
function(create_resources dir output)
    file(WRITE ${output} "")
    file(GLOB_RECURSE bins ${dir}/*)
    foreach(bin ${bins})
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        file(READ ${bin} filedata HEX)
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        file(APPEND ${output} "const unsigned char resource_${filename}[] = {${filedata}};\nconst unsigned long resource_${filename}_size = sizeof(resource_${filename});\n")
    endforeach()
endfunction()

set(TORC_BUILD_TYPE "${LIBONIONSHARE_BUILD_TYPE}")
add_subdirectory(external/torc)

include_directories(include)

create_resources(src/lib/resources src/lib/resources.h)
add_library(onionshare-object OBJECT src/lib/onionshare.c include/onionshare.h
        src/lib/templates.h src/lib/templates.cpp
        src/lib/resources.h
        src/lib/services.h src/lib/services.c
        src/lib/chat_service.c
        src/lib/include/nlohmann/json.hpp src/lib/include/inja.hpp
        src/lib/mongoose.h src/lib/mongoose.c)
target_include_directories(onionshare-object PUBLIC external/torc/src src/lib/include)

if(NOT LIBONIONSHARE_BUILD_TYPE STREQUAL "object")
    if(LIBONIONSHARE_BUILD_TYPE STREQUAL "shared")
        add_library(onionshare SHARED $<TARGET_OBJECTS:onionshare-object>)
    else()
        add_library(onionshare STATIC $<TARGET_OBJECTS:onionshare-object>)
    endif()

    target_link_libraries(onionshare torc)
    target_include_directories(onionshare PUBLIC external/torc/src src/lib/include)

    if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR) # only build CLI when not being used as a library/subdirectory
        add_executable(libonionshare_cli src/cli/main.c)
        target_link_libraries(libonionshare_cli onionshare)
    endif()
endif()