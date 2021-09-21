#include "menu-manager.hpp"

#include <QAction>
#include <QMenuBar>

#include <viewer-settings.hpp>

menu_manager::menu_manager(menu_bar *menubar)
        : QObject(nullptr) {
    auto file_menu = menubar->addMenu(tr("&File"));
    auto view_menu = menubar->addMenu(tr("&View"));
    auto property_menu = menubar->addMenu(tr("&Property"));
    auto window_menu = menubar->addMenu(tr("&Window"));
    auto help_menu = menubar->addMenu(tr("&Help"));
    auto file_menu_open = new QAction("Open");
    auto file_menu_open_dir = new QAction("Open directory");
    auto file_menu_close = new QAction("Close");
    auto file_menu_close_all = new QAction("Close All");
    auto file_menu_quit = new QAction("Quit");
    auto property_export = new QAction("Export");
    auto help_menu_about_qt = new QAction("About Qt");
    auto view_menu_word_wrap = new QAction("Word Wrap");
    auto window_menu_full_screen = new QAction("Full screen");
    help_menu->addAction(help_menu_about_qt);
    file_menu->addAction(file_menu_open);
    file_menu->addAction(file_menu_open_dir);
    file_menu->addSeparator();
    file_menu->addAction(file_menu_close);
    file_menu->addAction(file_menu_close_all);
    file_menu->addSeparator();
    file_menu->addAction(file_menu_quit);
    view_menu->addAction(view_menu_word_wrap);
    property_menu->addAction(property_export);
    window_menu->addAction(window_menu_full_screen);
    file_menu_close->setShortcut(QKeySequence::Close);
    file_menu_open->setShortcut(QKeySequence::Open);
    file_menu_quit->setShortcut(QKeySequence::Quit);
    window_menu_full_screen->setShortcut(QKeySequence::FullScreen);
    view_menu_word_wrap->setCheckable(true);
    file_menu_close->setIcon(QIcon::fromTheme("document-close"));
    file_menu_close_all->setIcon(QIcon::fromTheme("document-close"));
    file_menu_open->setIcon(QIcon::fromTheme("document-open"));
    file_menu_open_dir->setIcon(QIcon::fromTheme("folder-open"));
    file_menu_quit->setIcon(QIcon::fromTheme("application-exit"));
    help_menu_about_qt->setIcon(QIcon::fromTheme("help-about"));
    window_menu_full_screen->setIcon(QIcon::fromTheme("view-fullscreen"));
    window_menu_full_screen->setCheckable(true);
    file_menu_close->setEnabled(false);
    file_menu_close_all->setEnabled(false);

    viewer_settings settings;
    view_menu_word_wrap->setChecked(settings.view_word_wrap.value());
    window_menu_full_screen->setChecked(settings.window_show_fullscreen.value());

    connect(this, &menu_manager::use_word_wrap, [](auto &&value) {
        viewer_settings settings;
        settings.view_word_wrap.set(value);
    });

    connect(file_menu_quit, &action::triggered, this, &menu_manager::quit);
    connect(view_menu_word_wrap, &action::triggered, this, &menu_manager::use_word_wrap);
    connect(file_menu_close, &action::triggered, this, &menu_manager::close);
    connect(file_menu_close_all, &action::triggered, this, &menu_manager::close_all);
    connect(help_menu_about_qt, &action::triggered, this, &menu_manager::show_about_qt);
    connect(file_menu_open, &action::triggered, this, &menu_manager::open_file);
    connect(file_menu_open_dir, &action::triggered, this, &menu_manager::open_directory);
    connect(property_export, &action::triggered, this, &menu_manager::property_export);
    connect(window_menu_full_screen, &action::triggered, [this](bool value) {
        viewer_settings settings;
        settings.window_show_fullscreen.set(value);
        if (value)
            show_full_screen();
        else
            show_normal();
    });

    property_export->setIcon(QIcon::fromTheme("document-save"));

    m_close_action = file_menu_close;
    m_close_all_action = file_menu_close_all;
}

void menu_manager::set_close_enabled(const bool value) {
    m_close_action->setEnabled(value);
}

void menu_manager::set_close_all_enabled(const bool value) {
    m_close_all_action->setEnabled(value);
}
