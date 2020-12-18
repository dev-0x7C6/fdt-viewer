configure_file(config.h.in config.h)

add_subdirectory("submodules/qhexview")

add_executable(fdt-viewer
    endian-conversions.hpp
    fdt-header.hpp
    fdt-parser.cpp
    fdt-parser.hpp
    fdt-property-types.hpp
    main-window.cpp
    main-window.hpp
    menu-manager.cpp
    menu-manager.hpp
    main-window.ui
    dialogs.cpp
    dialogs.hpp
    fdt-view.cpp
    fdt-view.hpp
    main.cpp
    types.hpp
)

target_link_libraries(fdt-viewer PRIVATE Qt5::Widgets qhexview-lib)
install(TARGETS fdt-viewer RUNTIME DESTINATION bin)
