cmake_minimum_required( VERSION 3.5 )

project( fugio-sfalic LANGUAGES C CXX )

set( PLUGIN_NAME "sfalic" )
set( PLUGIN_DESCRIPTION "SFALIC - Simple Fast and Adaptive Lossless Image Compression algorithm" )
set( PLUGIN_FILENAME "${PROJECT_NAME}" )
set( PLUGIN_UUID "48f8fa15-cc9d-4031-82cf-71e4b4ec3363" )
set( PLUGIN_RELEASE_VERSION "1.0" )
set( PLUGIN_RELEASE_DATE "2019-10-20" )
set( PLUGIN_AUTHOR_NAME  "Alex May" )
set( PLUGIN_AUTHOR_EMAIL "bigfug@gmail.com" )

#------------------------------------------------------------------------------

set( CMAKE_INCLUDE_CURRENT_DIR ON )
set( CMAKE_AUTOUIC ON )
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_CXX_STANDARD 11)
set( CMAKE_CXX_STANDARD_REQUIRED ON )

#------------------------------------------------------------------------------
# Source files

add_library( ${PROJECT_NAME} MODULE
	include/fugio/sfalic/sfalic.h
	include/fugio/sfalic/uuid.h

	source/sfalicplugin.cpp
	source/sfalicplugin.h
	source/compressnode.cpp
	source/compressnode.h
	source/decompressnode.cpp
	source/decompressnode.h

	config.h.in
	manifest.json.in
)

target_include_directories( ${PROJECT_NAME} PRIVATE include )

set_target_properties( ${PROJECT_NAME} PROPERTIES
  BUNDLE TRUE
)

#------------------------------------------------------------------------------
# Installation targets

install( TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION .
)

install( FILES
	include/fugio/sfalic/sfalic.h
	include/fugio/sfalic/uuid.h
	DESTINATION include/fugio/sfalic
)

#------------------------------------------------------------------------------
# Plugin Config

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
	@ONLY
)

#------------------------------------------------------------------------------
# Plugin Manifest

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/manifest.json.in
	${CMAKE_CURRENT_BINARY_DIR}/manifest.json
	@ONLY
)

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/manifest.json DESTINATION . )

#------------------------------------------------------------------------------
# Fugio API

set( FUGIO_API "" CACHE PATH "" )

if( FUGIO_API AND EXISTS ${FUGIO_API} )
	target_include_directories( ${PROJECT_NAME} PRIVATE ${FUGIO_API} )

	target_sources( ${PROJECT_NAME} PRIVATE
		${FUGIO_API}/fugio/nodecontrolbase.h
		${FUGIO_API}/fugio/pincontrolbase.h
		)
else()
	message( "--- FUGIO_API NOT SET ---" )
endif()

#------------------------------------------------------------------------------
# Qt5

find_package( Qt5 COMPONENTS Core Widgets LinguistTools REQUIRED )

target_link_libraries( ${PROJECT_NAME} PRIVATE Qt5::Widgets)

#------------------------------------------------------------------------------
# Translations

set( TS_FILES
	translations/translations_de.ts
	)

qt5_create_translation( QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source ${TS_FILES} OPTIONS "-silent" )

configure_file( ${CMAKE_SOURCE_DIR}/translations/translations.qrc ${CMAKE_CURRENT_BINARY_DIR} COPYONLY )

qt5_add_resources( QRC_FILES ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc )

target_sources( ${PROJECT_NAME} PRIVATE
	${TS_FILES}
	${QM_FILES}
	${QRC_FILES}
	)

set_property( SOURCE ${CMAKE_CURRENT_BINARY_DIR}/qrc_translations.cpp PROPERTY SKIP_AUTOGEN ON )

#------------------------------------------------------------------------------
# SFALIC

target_include_directories( ${PROJECT_NAME} PRIVATE SFALIC )

set( SFALIC_SOURCES
	SFALIC/bigendian.c
	SFALIC/cdfpred.c
	SFALIC/cdfstat.c
	SFALIC/ceillog2.c
	SFALIC/cfamily.c
	SFALIC/clalloc.c
	SFALIC/decodecodeword.c
	SFALIC/tabrand.c
	SFALIC/cdftypes.h
	SFALIC/bigendian.h
	SFALIC/bppmask.h
	SFALIC/cdfpred.h
	SFALIC/cdfstat.h
	SFALIC/ceillog2.h
	SFALIC/cfamily.h
	SFALIC/clalloc.h
	SFALIC/decodecodeword.h
	SFALIC/encodecodeword.h
	SFALIC/str.h
	SFALIC/tabrand.h
	)

target_sources( ${PROJECT_NAME} PRIVATE ${SFALIC_SOURCES} )
