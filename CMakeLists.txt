cmake_minimum_required(VERSION 3.18)

project(fdt-viewer VERSION 0.3.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

configure_file(config.h.in config.h)

add_executable(fdt-viewer
    endian-conversions.hpp
    fdt-header.hpp
    fdt-parser.cpp
    fdt-parser.hpp
    fdt-property-types.hpp
    main-window.cpp
    main-window.hpp
    main-window.ui
    dialogs.cpp
    dialogs.hpp
    main.cpp
    types.hpp
)

target_link_libraries(fdt-viewer PRIVATE Qt5::Widgets)
install(TARGETS fdt-viewer RUNTIME DESTINATION bin)
