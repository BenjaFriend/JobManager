cmake_minimum_required ( VERSION 3.13 )

############### Project Settings ###############
project( "JobManager" )

#add_definitions( -D_CRT_SECURE_NO_WARNINGS )

# Set Build Dir
set( CMAKE_BINARY_DIR    "/build" )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_Output/$<CONFIG> )

# Sets the output directory
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin" )

message( STATUS "Using compiler:\t " ${CMAKE_CXX_COMPILER} )

############### Include Files  #################
# Glob include folder
include_directories(
    inc
    src
)

################# Complier Options ############
if( MSVC )
    set ( MY_COMPILER_FLAGS "/W3" )
else()
    set ( MY_COMPILER_FLAGS "-Wall -Wno-reorder -Wno-unknown-pragmas -Wno-multichar" )
endif()

set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MY_COMPILER_FLAGS}" )

### Setup visual studio source groups / filters ###
file( GLOB_RECURSE _source_list
    src/*.cpp* src/*.h* src/*.hpp*
    inc/*.cpp* inc/*.h* inc/*.hpp*
)

if( MSVC )
    foreach( _source IN ITEMS ${_source_list} )
    	get_filename_component( _source_path "${_source}" PATH )
        string( REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}" )
        string( REPLACE "/" "\\" _group_path "${_group_path}" )
        source_group( "${_group_path}" FILES "${_source}" )
    endforeach()
endif()


################# Add Exe ######################
message ( STATUS "\tcxx Flags:" ${CMAKE_CXX_FLAGS} )

add_executable( ${PROJECT_NAME} ${_source_list} )

#### Set C++ Standard
set_property( TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14 )




############### Link Libraries ################

# Link pthread if we are not using MSVC
if ( NOT MSVC )
    set( LINK_LIBS ${LINK_LIBS} pthread )
    message( STATUS "Added pthread!" )

    target_link_libraries( ${PROJECT_NAME} ${LINK_LIBS} )
endif()
