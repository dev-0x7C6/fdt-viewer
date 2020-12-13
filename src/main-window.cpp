#include "main-window.hpp"
#include "ui_main-window.h"

#include <QByteArray>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QMessageBox>
#include <QRegExp>

#include <dialogs.hpp>
#include <endian-conversions.hpp>
#include <fdt-parser.hpp>
#include <fdt-view.hpp>

using namespace Window;

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
    auto &&name = property.name;
    auto &&data = property.data;

    auto result = [&](string &&value) {
        return name + " = <" + value + ">;";
    };

    auto result_str = [&](string &&value) {
        return name + " = \"" + value + "\";";
    };

    if (property_map.contains(name)) {
        const property_info info = property_map.value(name);
        if (property_type::string == info.type)
            return result_str({data});

        if (property_type::number == info.type)
            return result(string::number(u32_be(data.data())));
    }

    const static regexp cells_regexp("^#.*-cells$");
    const static regexp names_regexp("^.*-names");

    if (cells_regexp.exactMatch(name))
        return result(string::number(u32_be(data.data())));

    if (names_regexp.exactMatch(name)) {
        auto lines = data.split(0);
        lines.removeLast();

        string ret;
        for (auto i = 0; i < lines.count(); ++i) {
            if (i == lines.count() - 1)
                ret += lines[i];
            else
                ret += lines[i] + ", ";
        }

        return result_str(std::move(ret));
    }

    if (std::count_if(data.begin(), data.end(), [](auto &&value) { return value == 0x00; }) == 1 &&
        data.back() == 0x00) return result_str({property.data});

    return result(present_u32be(property.data));
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
    connect(file_menu_open, &QAction::triggered, this, [this]() {
        fdt::open_file_dialog(this, [this](auto &&...values) { open_file(std::forward<decltype(values)>(values)...); });
    });
    connect(file_menu_open_dir, &QAction::triggered, this, [this]() {
        fdt::open_directory_dialog(this, [this](auto &&...values) { open_directory(std::forward<decltype(values)>(values)...); });
    });
    connect(file_menu_close, &QAction::triggered, this, &MainWindow::close);

    connect(m_ui->treeWidget, &QTreeWidget::itemClicked, [this](QTreeWidgetItem *item, auto...) {
        m_ui->textBrowser->clear();
        update_fdt_path(item);

        QVariant values = item->data(0, Qt::UserRole);
        auto name = item->data(0, Qt::DisplayRole).toString();
        auto properties = values.value<qt_fdt_properties>();

        string ret;
        ret.reserve(16 * 1024);
        ret += name + " {\n";
        for (auto &&property : properties)
            ret += "    " + present(property) + "\n";
        ret += "};";
        m_ui->textBrowser->setText(ret);
    });
}

void MainWindow::open_directory(const string &path) {
    QDirIterator iter(path, {"*.dtb", "*.dtbo"}, QDir::Files);
    while (iter.hasNext()) {
        iter.next();
        open_file(iter.filePath());
    }
}

void MainWindow::open_file(const string &path) {
    if (!open(path))
        QMessageBox::critical(this, tr("Invalid FDT format"), tr("Unable to parse %1").arg(path));
}

bool MainWindow::open(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    return fdt::fdt_view_prepare(m_ui->treeWidget, file.readAll(), {path});
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
