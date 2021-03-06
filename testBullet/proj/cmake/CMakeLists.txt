cmake_minimum_required( VERSION 2.8 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

project( testBullet )

get_filename_component( CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../Cinder" ABSOLUTE )
get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../" ABSOLUTE )

include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

set( SRC_FILES
	${APP_PATH}/src/testBulletApp.cpp
	${APP_PATH}/src/VehicleGeneric.h
	${APP_PATH}/src/VehicleGeneric.cpp
	${APP_PATH}/src/bullet/DrawablePlaneRBD.h
	${APP_PATH}/src/bullet/DrawablePlaneRBD.cpp
	${APP_PATH}/src/bullet/DrawableRBD.h
	${APP_PATH}/src/bullet/DrawableRBD.cpp
)

ci_make_app(
	SOURCES     ${SRC_FILES}
	INCLUDES    ${APP_PATH}/include
	CINDER_PATH ${CINDER_PATH}
	BLOCKS		${CINDER_PATH}/blocks/Cinder-
	BLOCKS		${CINDER_PATH}/blocks/Cinder-ImGui
	BLOCKS		${CINDER_PATH}/blocks/Cinder-Bullet3D
)
