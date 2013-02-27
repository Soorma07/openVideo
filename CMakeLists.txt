# CMakeLists.txt file for the libV4l2Capture project.
# Project can be found on github.
# See the documentation for more information.

#CMake required version.
cmake_minimum_required( VERSION 2.8 )

#Compilation options:
option ( USE_OPEN_CV
        "Use the openCV framework." ON )

# Set the project name:
project( libV4l2Capture )

# Set the project version number
set ( libV4l2Capture_VERSION_MAJOR 0 )
set ( libV4l2Capture_VERSION_MINOR 1 )

# Setup and configure the project header.
configure_file (
    "${PROJECT_SOURCE_DIR}/include/inFiles/libConfig.h.in"
    	"${PROJECT_BINARY_DIR}/include/libConfig.h"
    )
    
# Copy the header files to the binary tree
add_custom_command ( 
	TARGET libV4l2Capture POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${PROJECT_SOURCE_DIR}/libs/*"
				$<TARGET_FILE_DIR:libV4l2Capture> 
		)
		
# Add project source directories
add_subdirectory ( "${PROJECT_SOURCE_DIR}/src" )

# Add the binary tree to the search path for include
# files so we can find the library configuration.
include_directories ( "${PROJECT_BINARY_DIR}/include" )

# If we are using OpenCV
if ( USE_OPEN_CV )
    find_package( OpenCV REQUIRED )
    set ( EXTRA_LIBS ${EXTRA_LIBS} ${OpenCV_LIBS} )
endif ( USE_OPEN_CV )

# Set the version of the C++ standard
# If running from the command line, run cmake twice.
set ( CMAKE_CXX_FLAGS "-std=c++0x" )

# Add a build target (this will change to library when ready)
#This builds a runtime test, this is not related to gtest tests.
add_executable( libV4l2Capture  ${PROJECT_SOURCE_DIR}/src/openVideoTest.cpp )

# Add the libraries to build against for the target
target_link_libraries ( libV4l2Capture ${EXTRA_LIBS} )

