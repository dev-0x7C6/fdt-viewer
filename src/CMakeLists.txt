configure_file(config.h.in config.h)

add_subdirectory("submodules/qhexview")

file(GLOB_RECURSE fdt-viewer-headers *.hpp)
file(GLOB_RECURSE fdt-viewer-sources *.cpp)
file(GLOB_RECURSE fdt-viewer-uis *.ui)

add_executable(fdt-viewer
    ${fdt-viewer-headers}
    ${fdt-viewer-sources}
    ${fdt-viewer-uis}
    ../resources.qrc
)

target_link_libraries(fdt-viewer PRIVATE Qt6::Widgets Qt6::Core QScintilla QHexView)
install(TARGETS fdt-viewer RUNTIME DESTINATION bin)
