#include "dialogs.hpp"

#include <types.hpp>

#include <QFileDialog>
#include <QObject>

void fdt::open_file_dialog(widget *parent, path_callable &&callable) {
    const string_list filters{
        parent->tr("FDT files (*.dtb *.dtbo)"),
        parent->tr("FDT overlay files (*.dtbo)"),
        parent->tr("Any files (*.*)"),
    };

    QFileDialog dialog(parent);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setWindowTitle(parent->tr("Open Flattened Device Tree"));
    dialog.setDirectory(QDir::homePath());
    dialog.setNameFilter(filters.join(";;"));
    if (dialog.exec() == QDialog::Accepted)
        for (auto &&path : dialog.selectedFiles())
            callable(path);
}

void fdt::open_directory_dialog(widget *parent, path_callable &&callable) {
    QFileDialog dialog(parent);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setWindowTitle(parent->tr("Open Flattened Device Tree"));
    dialog.setDirectory(QDir::homePath());
    if (dialog.exec() == QDialog::Accepted)
        for (auto &&dir : dialog.selectedFiles())
            callable(dir);
}
