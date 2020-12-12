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
