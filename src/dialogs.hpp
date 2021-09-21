#pragma once

#include <types.hpp>

#include <functional>

#include <QByteArray>
#include <QString>

namespace dialogs {
auto ask_already_opened(widget *parent) noexcept -> bool;
auto warn_invalid_fdt(const string &filename, widget *parent) noexcept -> void;
} // namespace dialogs

namespace fdt {

using path_callable = std::function<void(const string &path)>;

void open_file_dialog(widget *parent, path_callable &&callable);
void open_directory_dialog(widget *parent, path_callable &&callable);
auto export_property_file_dialog(widget *parent, const QByteArray &data, const QString &hint) -> void;

} // namespace fdt
