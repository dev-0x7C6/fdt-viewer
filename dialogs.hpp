#pragma once

#include <types.hpp>

#include <functional>

namespace fdt {

using path_callable = std::function<void(const string &path)>;

void open_file_dialog(widget *parent, path_callable &&callable);
void open_directory_dialog(widget *parent, path_callable &&callable);

} // namespace fdt
