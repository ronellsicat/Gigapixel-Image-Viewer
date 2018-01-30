#set(Boost_USE_STATIC_LIBS		ON) # only find static libs

###################### CUDA SOURCES #######################

if(USE_CUDA)
set( SOURCES_CUDA
	# Put CUDA files (.cu) here.
)

set( INCLUDES_CUDA
	${CUDA_TOOLKIT_INCLUDE}
)

set( LINK_LIBS_CUDA
	${CUDA_CUDA_LIBRARY}	
	${CUDA_CUDART_LIBRARY}	
)

endif(USE_CUDA)

###################### IMAGE SOURCES #######################

set( SOURCES_IMAGE_SOURCES
	imagesources/imagesource.h
	imagesources/imagesource.cpp
	imagesources/tiledimage.h
	imagesources/tiledimage.cpp
	imagesources/imagedb.h
	imagesources/imagedb.cpp
)

###################### DRAWING / OPENGL SOURCES #######################

set( SOURCES_DRAWING
	drawing/texturecache.h
	drawing/drawtile.h
	drawing/drawtile.cpp
	drawing/drawonwindow.h
	drawing/drawonwindow.cpp
)

###################### TILED IMAGE EXPLORER #######################

set( SOURCES_TILED_IMAGE_EXPLORER
	tiledimageexplorer/tiledimageexplorer.h
	tiledimageexplorer/tiledimageexplorer.cpp

	tiledimageexplorer/tiledimagedata.h
	tiledimageexplorer/tiledimagedata.cpp
)

###################### IMAGE DB EXPLORER #######################


###################### ALL SOURCES #######################

set( SOURCES 
	main.cpp
	common.h
	common.cpp
	mainapplication.h
	mainapplication.cpp
	
	${SOURCES_IMAGE_SOURCES}

	${SOURCES_DRAWING}
	
	${SOURCES_TILED_IMAGE_EXPLORER}

	${SOURCES_CUDA}
)

#source_group( cudafiles FILES ${SOURCES_CUDA})

source_group( imagesources FILES ${SOURCES_IMAGE_SOURCES})

source_group( drawing FILES ${SOURCES_DRAWING})

source_group( tiledimageexplorer FILES ${SOURCES_TILED_IMAGE_EXPLORER})


include_directories( 
	
	external/ivda
	
	${INCLUDES_CUDA}
) 

link_directories(
	
	${Qt5_DIR}/lib
)

if(${USE_CUDA})
cuda_add_executable( GigaPatchExplorer
	${SOURCES} 
)
else(${USE_CUDA})
add_executable( GigaPatchExplorer
	${SOURCES} 
)
endif(${USE_CUDA})

# These libraries are linked for Release and Debug mode.
target_link_libraries( GigaPatchExplorer 
	Qt5::Widgets

	${LINK_LIBS_CUDA}

	#debug Qt5Qmld optimized Qt5Qml
	#debug Qt5Quickd optimized Qt5Quick
	#debug Qt5PrintSupportd optimized Qt5PrintSupport
	
	debug ${CMAKE_CURRENT_SOURCE_DIR}/external/ivda/ivdatoolsd.lib 
	optimized ${CMAKE_CURRENT_SOURCE_DIR}/external/ivda/ivdatools.lib
)

# Copy DLLs:
# TODO: Copy Debug or Release versions depending on build type to save memory.
#string(REPLACE "." "" opencv_version_nodots ${OpenCV_VERSION})
#string(REPLACE ".lib" ".dll" Boost_SERIALIZATION_DLL_DEBUG ${Boost_SERIALIZATION_LIBRARY_DEBUG})
#string(REPLACE "/libboost" "/boost" Boost_SERIALIZATION_DLL_DEBUG ${Boost_SERIALIZATION_DLL_DEBUG})
#string(REPLACE ".lib" ".dll" Boost_SERIALIZATION_DLL_RELEASE ${Boost_SERIALIZATION_LIBRARY_RELEASE})
#string(REPLACE "/libboost" "/boost" Boost_SERIALIZATION_DLL_RELEASE ${Boost_SERIALIZATION_DLL_RELEASE})
set ( DLLs_TO_COPY 

	${Qt5_DIR}/bin/icudt53.dll
	${Qt5_DIR}/bin/icuin53.dll
	${Qt5_DIR}/bin/icuuc53.dll
	${Qt5_DIR}/bin/Qt5Cored.dll
	${Qt5_DIR}/bin/Qt5Core.dll
	${Qt5_DIR}/bin/Qt5Guid.dll
	${Qt5_DIR}/bin/Qt5Gui.dll
	${Qt5_DIR}/bin/Qt5Widgetsd.dll
	${Qt5_DIR}/bin/Qt5Widgets.dll
#	${Qt5_DIR}/bin/Qt5PrintSupportd.dll
#	${Qt5_DIR}/bin/Qt5PrintSupport.dll
) 

# Do the copying here:
foreach( DLLNAME ${DLLs_TO_COPY} )	
	if( EXISTS ${DLLNAME} )
		add_custom_command( TARGET GigaPatchExplorer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DLLNAME} $<TARGET_FILE:GigaPatchExplorer>/..)
		message( "Copying " ${DLLNAME} " to " "${EXE_NAME}/.." )
	endif( EXISTS ${DLLNAME} )
endforeach( DLLNAME )