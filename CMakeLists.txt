cmake_minimum_required(VERSION 3.26)

project(fdt-viewer VERSION 0.8.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets Core)
if (Qt6_FOUND)
	add_subdirectory("src")

	if (UNIX)
		install(FILES resources/fdt-viewer.svg DESTINATION share/icons/hicolor/scalable/apps)
		install(FILES resources/fdt-viewer.desktop DESTINATION share/applications)
	endif (UNIX)
endif()

find_package(Git REQUIRED)
# the commit's SHA1, and whether the building workspace was dirty or not
execute_process(COMMAND
        "${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --dirty
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_SHA1
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}-${GIT_SHA1}")
set(CPACK_PACKAGE_VENDOR "Bartłomiej Burdukiewicz")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dmitry Mikushin <dmitry@kernelgen.org>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS libqt5gui5)
set(CPACK_PACKAGING_INSTALL_PREFIX "usr")

add_custom_target(docker
        COMMAND docker build -f docker/Dockerfile -t ${PROJECT_NAME} .
        COMMAND docker create --name ${CPACK_PACKAGE_FILE_NAME} ${PROJECT_NAME}
	COMMAND docker cp ${CPACK_PACKAGE_FILE_NAME}:/root/build/${CPACK_PACKAGE_FILE_NAME}.deb ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND docker rm -f ${CPACK_PACKAGE_FILE_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building ${PROJECT_NAME} in a Docker container")

include(CPack)

