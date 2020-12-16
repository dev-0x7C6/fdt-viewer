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

constexpr auto BINARY_PREVIEW_LIMIT = 2048;

string present_u32be(const QByteArray &data) {
    string ret;

    auto array = reinterpret_cast<u8 *>(const_cast<char *>(data.data()));
    for (auto i = 0; i < data.size(); ++i) {
        ret += "0x" + QString::number(array[i], 16).rightJustified(2, '0').toUpper() + " ";
        if (i == BINARY_PREVIEW_LIMIT) {
            ret += "... ";
            break;
        }
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
    auto view_menu = m_ui->menubar->addMenu(tr("&View"));
    auto help_menu = m_ui->menubar->addMenu(tr("&Help"));
    auto file_menu_open = new QAction("Open");
    auto file_menu_open_dir = new QAction("Open directory");
    auto file_menu_close = new QAction("Close");
    auto file_menu_quit = new QAction("Quit");
    auto help_menu_about_qt = new QAction("About Qt");
    auto view_menu_word_wrap = new QAction("Word Wrap");
    help_menu->addAction(help_menu_about_qt);
    file_menu->addAction(file_menu_open);
    file_menu->addAction(file_menu_open_dir);
    file_menu->addSeparator();
    file_menu->addAction(file_menu_close);
    file_menu->addSeparator();
    file_menu->addAction(file_menu_quit);
    view_menu->addAction(view_menu_word_wrap);
    file_menu_close->setShortcut(QKeySequence::Close);
    file_menu_open->setShortcut(QKeySequence::Open);
    file_menu_quit->setShortcut(QKeySequence::Quit);
    view_menu_word_wrap->setCheckable(true);
    file_menu_close->setIcon(QIcon::fromTheme("document-close"));
    file_menu_open->setIcon(QIcon::fromTheme("document-open"));
    file_menu_open_dir->setIcon(QIcon::fromTheme("folder-open"));
    file_menu_quit->setIcon(QIcon::fromTheme("application-exit"));
    help_menu_about_qt->setIcon(QIcon::fromTheme("help-about"));

    m_ui->text_view->setWordWrapMode(QTextOption::NoWrap);

    connect(view_menu_word_wrap, &QAction::triggered, [this, view_menu_word_wrap]() {
        m_ui->text_view->setWordWrapMode(view_menu_word_wrap->isChecked() ? QTextOption::WordWrap : QTextOption::NoWrap);
    });

    connect(help_menu_about_qt, &QAction::triggered, []() { QApplication::aboutQt(); });

    connect(file_menu_open, &QAction::triggered, this, [this]() {
        fdt::open_file_dialog(this, [this](auto &&...values) { open_file(std::forward<decltype(values)>(values)...); });
    });
    connect(file_menu_open_dir, &QAction::triggered, this, [this]() {
        fdt::open_directory_dialog(this, [this](auto &&...values) { open_directory(std::forward<decltype(values)>(values)...); });
    });

    connect(file_menu_quit, &QAction::triggered, this, &MainWindow::close);
    connect(file_menu_close, &QAction::triggered, this, [this]() {
        if (m_fdt) {
            delete m_fdt;
            m_fdt = nullptr;
            m_ui->text_view->clear();
            m_ui->statusbar->clearMessage();
            m_ui->path->clear();
            update_view();
        }
    });

    connect(m_ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, &MainWindow::update_view);
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

void MainWindow::render(tree_widget_item *item, string &ret, int depth) {
    string depth_str;
    depth_str.fill(' ', depth * 4);

    QList<tree_widget_item *> nodes;
    QList<tree_widget_item *> properties;

    for (auto i = 0; i < item->childCount(); ++i) {
        const auto child = item->child(i);
        switch (child->data(0, QT_ROLE_NODETYPE).value<NodeType>()) {
            case NodeType::Node:
                nodes.append(child);
                break;
            case NodeType::Property:
                properties.append(child);
                break;
        }
    }

    ret += depth_str + item->data(0, Qt::DisplayRole).toString() + " {\n";

    for (auto item : properties) {
        const auto property = item->data(0, QT_ROLE_PROPERTY).value<qt_fdt_property>();
        ret += depth_str + "    " + present(property) + "\n";
    }

    if (!properties.isEmpty() && !nodes.isEmpty())
        ret += "\n";

    for (auto i = 0; i < nodes.count(); ++i) {
        render(nodes.at(i), ret, depth + 1);

        if (nodes.count() - 1 != i)
            ret += "\n";
    }

    ret += depth_str + "};\n";
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

    m_fdt = root;

    m_ui->statusbar->showMessage("file://" + root->data(0, QT_ROLE_FILEPATH).toString());
    m_ui->path->setText("fdt://" + path);
}

constexpr auto VIEW_TEXT_CACHE_SIZE = 1024 * 1024;

void MainWindow::update_view() {
    if (m_ui->treeWidget->selectedItems().isEmpty())
        return;

    const auto item = m_ui->treeWidget->selectedItems().first();

    const auto type = item->data(0, QT_ROLE_NODETYPE).value<NodeType>();
    m_ui->preview->setCurrentWidget(NodeType::Node == type ? m_ui->text_view_page : m_ui->property_view_page);

    if (NodeType::Property == type) {
        const auto property = item->data(0, QT_ROLE_PROPERTY).value<qt_fdt_property>();
        m_ui->property_as_string_value->setText({property.data.data()});
    }

    m_ui->text_view->clear();
    update_fdt_path(item);

    string ret;
    ret.reserve(VIEW_TEXT_CACHE_SIZE);

    render(item, ret);
    m_ui->text_view->setText(ret);
}

MainWindow::~MainWindow() = default;
