project (testcheckheaders)

set (SRCS
    testrunner.cpp
    testsuite.cpp
    testwarningincludeheaders.cpp
    ../src/checkheaders.cpp
    ../src/commoncheck.cpp
    ../src/filelister.cpp
    ../src/FileParser.cpp
    ../src/tokenize.cpp)

include_directories(../src)

add_executable (testcheckheaders ${SRCS})

if(MSVC)
    target_link_libraries(testcheckheaders shlwapi)
endif(MSVC)
