# Widget gallery for comparing Phantom style variants.
#
# Builds one executable gallery_<variant> per style variant found in VARIANTS_DIR
# (each subdirectory contains phantomstyle.{h,cpp}, phantomcolor.{h,cpp}, phantomtweak.h)
# plus gallery_fusion (Qt's built-in Fusion style, for reference).
#
# Normally invoked by render_shots.py.

cmake_minimum_required( VERSION 3.16 )
project( PhantomStyleGallery CXX )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_AUTOMOC ON )
find_package( Qt6 COMPONENTS Core Gui Widgets REQUIRED )

set( VARIANTS_DIR "${CMAKE_CURRENT_BINARY_DIR}/variants" CACHE PATH "Directory with one subdirectory per style variant" )

file( GLOB VARIANT_DIRS LIST_DIRECTORIES true "${VARIANTS_DIR}/*" )
foreach( VDIR ${VARIANT_DIRS} )
	if( IS_DIRECTORY ${VDIR} AND EXISTS ${VDIR}/phantomstyle.cpp )
		get_filename_component( V ${VDIR} NAME )
		add_executable( gallery_${V}
			gallery.cpp
			${VDIR}/phantomstyle.cpp
			${VDIR}/phantomcolor.cpp
			${VDIR}/phantomstyle.h
		)
		target_include_directories( gallery_${V} PRIVATE ${VDIR} )
		target_link_libraries( gallery_${V} Qt6::Widgets )
	endif()
endforeach()

add_executable( gallery_fusion gallery.cpp )
target_compile_definitions( gallery_fusion PRIVATE USE_FUSION )
target_link_libraries( gallery_fusion Qt6::Widgets )
