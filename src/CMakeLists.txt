configure_file(config.h.in config.h)

add_subdirectory("submodules/qhexview")

add_executable(fdt-viewer
    dialogs.cpp
    dialogs.hpp
    endian-conversions.hpp
    fdt-header.hpp
    fdt-parser.cpp
    fdt-parser.hpp
    fdt-property-types.hpp
    fdt-view.cpp
    fdt-view.hpp
    main-window.cpp
    main-window.hpp
    main-window.ui
    main.cpp
    menu-manager.cpp
    menu-manager.hpp
    types.hpp
    viewer-settings.cpp
    viewer-settings.hpp
)

target_link_libraries(fdt-viewer PRIVATE Qt5::Widgets qhexview-lib)
install(TARGETS fdt-viewer RUNTIME DESTINATION bin)
