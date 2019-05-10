# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required ( VERSION 3.13 )
set( CMAKE_SUPPRESS_REGENERATION true )

project ( "JobManager" )

# Include sub-projects.
add_subdirectory ( "JobManager" )
