configure_file(config.h.in config.h)

add_subdirectory("submodules/qhexview")

add_executable(fdt-viewer
    dialogs.cpp
    dialogs.hpp
    endian-conversions.hpp
    fdt/fdt-generator-qt.cpp
    fdt/fdt-generator-qt.hpp
    fdt/fdt-generator.hpp
    fdt/fdt-header.hpp
    fdt/fdt-parser.cpp
    fdt/fdt-parser.hpp
    fdt/fdt-property-types.hpp
    fdt/fdt-view.cpp
    fdt/fdt-view.hpp
    main-window.cpp
    main-window.hpp
    main-window.ui
    main.cpp
    menu-manager.cpp
    menu-manager.hpp
    types.hpp
    viewer-settings.cpp
    viewer-settings.hpp
    ../resources.qrc
)

target_link_libraries(fdt-viewer PRIVATE Qt6::Widgets Qt6::Core QHexView)
install(TARGETS fdt-viewer RUNTIME DESTINATION bin)
