# Add unit test source directories
add_subdirectory ( "${PROJECT_SOURCE_DIR}/gtest" ${PROJECT_BINARY_DIR}/tests )

# Add gtest to the project
add_subdirectory( /usr/src/gtest ${PROJECT_BINARY_DIR}/gtest )
include_directories( "/usr/include/gtest" )
set ( EXTRA_LIBS ${EXTRA_LIBS} gtest gtest_main pthread )

# Add gtest executables
add_executable( libV4l2CaptureTests ${PROJECT_SOURCE_DIR}/gtest/unitTest.cpp )

# Add the libraries to build against for the target
target_link_libraries ( libV4l2CaptureTests ${EXTRA_LIBS} )