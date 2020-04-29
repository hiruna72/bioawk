# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.")
endif()

# choose the architecture
set(DEPLOY_PLATFORM "unknown" CACHE STRING "choose the instruction set architecture")
set_property(CACHE DEPLOY_PLATFORM PROPERTY STRINGS unknown x86 arm64-v8a armeabi-v7a)
IF (${DEPLOY_PLATFORM} STREQUAL "unknown")
    message(FATAL_ERROR "choose the DEPLOY_PLATFORM")
    return() # This is to stop proceeding further and to stop opencv getting set to the default ANDROID_ABI
ENDIF()

# Set minimum cmake version + cmake settings
cmake_minimum_required (VERSION 3.9)

# Specify the project, and set the use of c++ and c
project(BIOAWK LANGUAGES CXX C)

message(STATUS "Configure BIOAWK commit fd40150b7c557da45e781a999d372abbc634cc21")

# Set project name
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC -Wall -O3 -std=c11 -fsigned-char -D_FILE_OFFSET_BITS=64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3  -std=c++11 -fopenmp -fsigned-char -D_FILE_OFFSET_BITS=64")

include_directories(${PROJECT_SOURCE_DIR})

set(ytab ytab.c)
set(b b.c)
set(main main.c)
set(parse parse.c)
set(proctab proctab.c)
set(tran tran.c)
set(lib lib.c)
set(run run.c)
set(lex lex.c)
set(addon addon.c)



#message(STATUS "C_SOURCES=${C_SOURCES}")
#message(STATUS "CPP_SOURCES=${CPP_SOURCES}")


#add_executable(bioawk ${ytab} ${b} ${main} ${parse} ${proctab} ${tran} ${lib} ${run} ${lex} ${addon})
add_library(bioawk SHARED ${ytab} ${b} ${main} ${parse} ${proctab} ${tran} ${lib} ${run} ${lex} ${addon})

# link with dependencies
target_link_libraries(bioawk -lz -lm)

# Create the executable
add_executable(bioawk_example examples/init_bioawk_example.c)

# Link the required libraries
target_link_libraries(bioawk_example bioawk)