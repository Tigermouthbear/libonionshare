cmake_minimum_required(VERSION 3.17)
project(libonionshare C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

# Creates C resources file from files in given directory
function(create_resources dir output)
    # Create empty output file
    file(WRITE ${output} "")
    # Collect input files
    file(GLOB_RECURSE bins ${dir}/*)
    # Iterate through input files
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND ${output} "const unsigned char resource_${filename}[] = {${filedata}};\nconst unsigned long resource_${filename}_size = sizeof(resource_${filename});\n")
    endforeach()
endfunction()

add_subdirectory(external/torc)
add_subdirectory(external/jinja2cpp)
include_directories(include)

create_resources(src/lib/resources src/lib/resources.h)
add_library(onionshare src/lib/onionshare.c include/onionshare.h src/lib/templates.h src/lib/templates.cpp src/lib/resources.h src/lib/mongoose.h src/lib/mongoose.c src/lib/services.h src/lib/services.c src/lib/chat_service.c)
target_link_libraries(onionshare torc jinja2cpp)
target_include_directories(onionshare PUBLIC external/torc/src external/jinja2cpp/include)

add_executable(libonionshare_cli src/cli/main.c)
target_link_libraries(libonionshare_cli onionshare)
