cmake_minimum_required( VERSION 2.8.11 )

project( GigaPatchExplorer )

# Find includes in corresponding build directories
set( CMAKE_INCLUDE_CURRENT_DIR ON )

# Instruct CMake to run moc automatically when needed.
set( CMAKE_AUTOMOC ON )

# Set up CUDA:
set( USE_CUDA OFF CACHE BOOL "Use CUDA or not." )
message( "CUDA is " ${USE_CUDA})
if( ${USE_CUDA} )
find_package( Cuda REQUIRED )
add_definitions(-DGIGAPATCHEXPLORER_USE_CUDA)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_35,code=sm_35)
endif( ${USE_CUDA} )

# Set up Qt5:
set( Qt5_DIR "" CACHE PATH "Directory where Qt5 is installed (contains bin, include, lib folders)." )
set( CMAKE_PREFIX_PATH ${Qt5_DIR}/lib/cmake/Qt5Widgets )
find_package( Qt5Widgets REQUIRED )

message( "\nIMPORTANT: If you encounter Qt5 related errors (e.g. looking for Qt5Widgets_DIR), 
	make sure to set Qt5_DIR then run Configure!\n")
message( "\nIMPORTANT: If you are building a VS solution, make sure to set the startup project to
	GigaPatchExplorer to enable debugging.\n")

# Set up OpenCV:
#find_package( OpenCV REQUIRED )

# Set up Boost:
#set(Boost_USE_STATIC_LIBS		ON) # only find static libs
#set(Boost_USE_MULTITHREADED     ON)
#find_package( Boost REQUIRED COMPONENTS serialization)

add_subdirectory( src )