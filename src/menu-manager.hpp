#pragma once

#include <types.hpp>
#include <QObject>

class menu_manager : public QObject {
    Q_OBJECT
public:
    menu_manager(menu_bar *menubar);

public:
    void set_close_enabled(bool value);
    void set_close_all_enabled(bool value);

signals:
    void open_file();
    void open_directory();
    void close();
    void close_all();
    void quit();
    void property_export();

    void show_about_qt();

    void use_word_wrap(bool);

    void show_full_screen();
    void show_normal();

private:
    action *m_close_action{nullptr};
    action *m_close_all_action{nullptr};
};
