#include "dialogs.hpp"

#include <types.hpp>

#include <QFileDialog>
#include <QMessageBox>
#include <QObject>

void fdt::open_file_dialog(widget *parent, path_callable &&callable) {
    const string_list filters{
        parent->tr("FDT formats (*.dtb *.dtbo *.itb)"),
        parent->tr("FIT container (*.itb)"),
        parent->tr("FDT file (*.dtb)"),
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

auto dialogs::ask_already_opened(widget *parent) noexcept -> bool {
    return QMessageBox::question(parent, parent->tr("Question"), parent->tr("File is already opened, do you want to reload?"), QMessageBox::Yes | QMessageBox::No) !=
        QMessageBox::Yes;
}

auto dialogs::warn_invalid_fdt(const string &filename, widget *parent) noexcept -> void {
    QMessageBox::critical(parent, parent->tr("Invalid FDT format"), parent->tr("Unable to parse %1").arg(filename));
}

auto fdt::export_property_file_dialog(widget *parent, const QByteArray &data, const QString &hint) -> void {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QFileDialog::saveFileContent(data, hint);
#else
    QFileDialog *dialog = new QFileDialog();
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->selectFile(hint);

    auto fileSelected = [=](const QString &fileName) {
        if (!fileName.isNull()) {
            QFile selectedFile(fileName);
            if (selectedFile.open(QIODevice::WriteOnly))
                selectedFile.write(data);
        }
    };

    auto dialogClosed = [=](int code) {
        Q_UNUSED(code);
        dialog->deleteLater();
    };

    QObject::connect(dialog, &QFileDialog::fileSelected, fileSelected);
    QObject::connect(dialog, &QFileDialog::finished, dialogClosed);
    dialog->show();
#endif
}
