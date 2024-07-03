#pragma once

#include <types.hpp>

#include <functional>

#include <QByteArray>
#include <QString>

class QWidget;

namespace dialogs {
auto ask_already_opened(QWidget *parent) noexcept -> bool;
auto warn_invalid_fdt(const QString &filename, QWidget *parent) noexcept -> void;
} // namespace dialogs

namespace fdt {

using path_callable = std::function<void(const QString &path)>;

void open_file_dialog(QWidget *parent, path_callable &&callable);
void open_directory_dialog(QWidget *parent, path_callable &&callable);
auto export_property_file_dialog(QWidget *parent, const QByteArray &data, const QString &hint) -> void;

} // namespace fdt
