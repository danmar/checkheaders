project (checkheaders)

cmake_minimum_required (VERSION 2.8)

set (SRCS
    src/main.cpp
    src/checkheaders.cpp
    src/commoncheck.cpp
    src/filelister.cpp
    src/tokenize.cpp
    src/FileParser.cpp)

add_executable (checkheaders ${SRCS})

if(MSVC)
	target_link_libraries(checkheaders shlwapi)
endif(MSVC)

install (TARGETS checkheaders RUNTIME DESTINATION bin)

add_subdirectory(test)
