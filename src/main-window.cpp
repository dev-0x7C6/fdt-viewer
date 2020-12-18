#include "main-window.hpp"
#include "ui_main-window.h"

#include <QByteArray>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QMessageBox>
#include <QTreeWidget>

#include <dialogs.hpp>
#include <endian-conversions.hpp>
#include <fdt-parser.hpp>
#include <fdt-view.hpp>

#include <document/qhexdocument.h>
#include <document/buffer/qmemorybuffer.h>
#include <qhexview.h>

using namespace Window;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , m_ui(std::make_unique<Ui::MainWindow>()) {
    m_ui->setupUi(this);
    m_ui->preview->setCurrentWidget(m_ui->text_view_page);
    m_ui->splitter->setEnabled(false);

    m_hexview = new QHexView();
    m_hexview->setReadOnly(true);
    m_ui->hexview_layout->addWidget(m_hexview);

    m_ui->splitter->setStretchFactor(0, 2);
    m_ui->splitter->setStretchFactor(1, 5);

    auto file_menu = m_ui->menubar->addMenu(tr("&File"));
    auto view_menu = m_ui->menubar->addMenu(tr("&View"));
    auto help_menu = m_ui->menubar->addMenu(tr("&Help"));
    auto file_menu_open = new QAction("Open");
    auto file_menu_open_dir = new QAction("Open directory");
    auto file_menu_close = new QAction("Close");
    auto file_menu_close_all = new QAction("Close All");
    auto file_menu_quit = new QAction("Quit");
    auto help_menu_about_qt = new QAction("About Qt");
    auto view_menu_word_wrap = new QAction("Word Wrap");
    help_menu->addAction(help_menu_about_qt);
    file_menu->addAction(file_menu_open);
    file_menu->addAction(file_menu_open_dir);
    file_menu->addSeparator();
    file_menu->addAction(file_menu_close);
    file_menu->addAction(file_menu_close_all);
    file_menu->addSeparator();
    file_menu->addAction(file_menu_quit);
    view_menu->addAction(view_menu_word_wrap);
    file_menu_close->setShortcut(QKeySequence::Close);
    file_menu_open->setShortcut(QKeySequence::Open);
    file_menu_quit->setShortcut(QKeySequence::Quit);
    view_menu_word_wrap->setCheckable(true);
    file_menu_close->setIcon(QIcon::fromTheme("document-close"));
    file_menu_close_all->setIcon(QIcon::fromTheme("document-close"));
    file_menu_open->setIcon(QIcon::fromTheme("document-open"));
    file_menu_open_dir->setIcon(QIcon::fromTheme("folder-open"));
    file_menu_quit->setIcon(QIcon::fromTheme("application-exit"));
    help_menu_about_qt->setIcon(QIcon::fromTheme("help-about"));

    m_file_close_action = file_menu_close;
    m_file_close_all_action = file_menu_close_all;

    m_file_close_action->setEnabled(false);
    m_file_close_all_action->setEnabled(false);

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
            update_view();
        }
    });

    connect(file_menu_close_all, &QAction::triggered, this, [this]() {
        m_fdt = nullptr;
        m_ui->treeWidget->clear();
        update_view();
    });

    connect(m_ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, &MainWindow::update_view);
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

bool MainWindow::open(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    const auto ret = fdt::fdt_view_prepare(m_ui->treeWidget, file.readAll(), {path});
    update_view();
    return ret;
}

void MainWindow::update_fdt_path(QTreeWidgetItem *item) {
    m_file_close_action->setEnabled(item);

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
    m_ui->splitter->setEnabled(m_ui->treeWidget->topLevelItemCount());
    m_file_close_action->setEnabled(!m_ui->treeWidget->selectedItems().isEmpty());
    m_file_close_all_action->setEnabled(m_ui->treeWidget->topLevelItemCount());

    if (m_ui->treeWidget->selectedItems().isEmpty()) {
        m_ui->preview->setCurrentWidget(m_ui->text_view_page);
        m_ui->text_view->clear();
        m_ui->statusbar->clearMessage();
        m_ui->path->clear();
        return;
    }

    const auto item = m_ui->treeWidget->selectedItems().first();

    const auto type = item->data(0, QT_ROLE_NODETYPE).value<NodeType>();
    m_ui->preview->setCurrentWidget(NodeType::Node == type ? m_ui->text_view_page : m_ui->property_view_page);

    if (NodeType::Property == type) {
        const auto property = item->data(0, QT_ROLE_PROPERTY).value<qt_fdt_property>();
        m_hexview->setDocument(QHexDocument::fromMemory<QMemoryBuffer>(property.data));
    }

    m_ui->text_view->clear();
    update_fdt_path(item);

    string ret;
    ret.reserve(VIEW_TEXT_CACHE_SIZE);

    fdt::fdt_view_dts(item, ret);
    m_ui->text_view->setText(ret);
}

MainWindow::~MainWindow() = default;
