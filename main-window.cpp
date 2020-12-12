#include "main-window.hpp"
#include "ui_main-window.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>

#include <endian-conversions.hpp>
#include <fdt-parser.hpp>

#include <iostream>
#include <stack>

using namespace Window;

struct qt_fdt_property {
    QString name;
    QByteArray data;
};

using qt_fdt_properties = QList<qt_fdt_property>;

Q_DECLARE_METATYPE(qt_fdt_property)
Q_DECLARE_METATYPE(qt_fdt_properties)

constexpr auto QT_ROLE_PROPERTY = Qt::UserRole;
constexpr auto QT_ROLE_FILEPATH = Qt::UserRole + 1;

string present_u32be(const QByteArray &data) {
    string ret;

    auto array = reinterpret_cast<u8 *>(const_cast<char *>(data.data()));
    for (auto i = 0; i < data.size(); ++i) {
        ret += "0x" + QString::number(array[i], 16).rightJustified(2, '0').toUpper() + " ";
    }
    ret.remove(ret.size() - 1, 1);

    return ret;
}

string present(const qt_fdt_property &property) {
    string ret;
    ret += property.name + " = ";
    ret += "<" + present_u32be(property.data) + ">;";
    return ret;
}

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , m_ui(std::make_unique<Ui::MainWindow>()) {
    m_ui->setupUi(this);

    m_ui->splitter->setStretchFactor(0, 2);
    m_ui->splitter->setStretchFactor(1, 5);

    auto file_menu = m_ui->menubar->addMenu(tr("&File"));
    auto file_menu_open = new QAction("Open");
    auto file_menu_open_dir = new QAction("Open directory");
    auto file_menu_close = new QAction("Close");
    file_menu->addAction(file_menu_open);
    file_menu->addAction(file_menu_open_dir);
    file_menu->addAction(file_menu_close);
    file_menu_open->setShortcut(QKeySequence::Open);
    file_menu_close->setShortcut(QKeySequence::Close);
    connect(file_menu_open, &QAction::triggered, this, &MainWindow::open_file_dialog);
    connect(file_menu_open_dir, &QAction::triggered, this, &MainWindow::open_directory_dialog);
    connect(file_menu_close, &QAction::triggered, this, &MainWindow::close);

    connect(m_ui->treeWidget, &QTreeWidget::itemClicked, [this](QTreeWidgetItem *item, auto...) {
        m_ui->textBrowser->clear();
        update_fdt_path(item);

        QVariant values = item->data(0, Qt::UserRole);
        auto properties = values.value<qt_fdt_properties>();
        for (auto &&property : properties) {
            m_ui->textBrowser->append(present(property));
        }
    });
}

void MainWindow::open_file_dialog() {
    const QStringList filters{
        tr("FDT files (*.dtb *.dtbo)"),
        tr("FDT overlay files (*.dtbo)"),
        tr("Any files (*.*)"),
    };

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setWindowTitle(tr("Open Flattened Device Tree"));
    dialog.setDirectory(QDir::homePath());
    dialog.setNameFilter(filters.join(";;"));
    if (dialog.exec() == QDialog::Rejected)
        return;

    for (auto &&path : dialog.selectedFiles())
        if (!open(path))
            QMessageBox::critical(this, tr("Invalid FDT format"), tr("Unable to parse %1").arg(path));
}

void MainWindow::open_directory_dialog() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setWindowTitle(tr("Open Flattened Device Tree"));
    dialog.setDirectory(QDir::homePath());
    if (dialog.exec() == QDialog::Rejected)
        return;

    for (auto &&dir : dialog.selectedFiles()) {
        QDirIterator iter(dir, {"*.dtb", "*.dtbo"}, QDir::Files, QDirIterator::Subdirectories);
        while (iter.hasNext()) {
            iter.next();
            if (!open(iter.filePath()))
                QMessageBox::critical(this, tr("Invalid FDT format"), tr("Unable to parse %1").arg(iter.filePath()));
        }
    }
}

bool MainWindow::open(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QFileInfo info(path);
    auto datamap = file.readAll();

    fdt_generator generator;

    auto root = new QTreeWidgetItem(m_ui->treeWidget);
    root->setText(0, info.fileName());
    root->setData(0, QT_ROLE_FILEPATH, info.absoluteFilePath());

    std::stack<QTreeWidgetItem *> tree_stack;

    generator.begin_node = [&](std::string_view &&name) {
        auto child = [&]() {
            if (tree_stack.empty())
                return root;
            else
                return new QTreeWidgetItem(tree_stack.top());
        }();

        if (child->text(0).isEmpty())
            child->setText(0, QString::fromStdString(name.data()));

        tree_stack.emplace(child);
    };

    generator.end_node = [&tree_stack]() {
        tree_stack.pop();
    };

    generator.insert_property = [&tree_stack](std::string_view &&name, std::string_view &&data) {
        auto current = tree_stack.top();
        QVariant values = current->data(0, QT_ROLE_PROPERTY);
        auto properties = values.value<qt_fdt_properties>();
        qt_fdt_property property;
        property.name = QString::fromStdString(name.data());
        property.data = QByteArray(data.data(), data.size());
        properties << property;
        current->setData(0, QT_ROLE_PROPERTY, QVariant::fromValue(properties));
    };

    fdt_parser parser(datamap.data(), datamap.size(), generator);

    if (!parser.is_valid()) {
        delete root;
        return false;
    }

    return true;
}

void MainWindow::update_fdt_path(QTreeWidgetItem *item) {
    if (nullptr == item) {
        m_ui->path->clear();
        return;
    }

    QString path = item->text(0);
    auto root = item;
    while (root->parent()) {
        path = root->parent()->text(0) + "/" + path;
        root = root->parent();
    }

    m_ui->statusbar->showMessage("file://" + root->data(0, QT_ROLE_FILEPATH).toString());
    m_ui->path->setText("fdt://" + path);
}

MainWindow::~MainWindow() = default;
