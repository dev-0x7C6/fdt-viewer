#pragma once

#include <QRect>
#include <QSettings>
#include <QVariant>

#include <types.hpp>

using settings = QSettings;

template <typename type>
class settings_property {
public:
    settings_property(const string &name, type &&value = {})
            : m_name(name) {
        if (!m_settings.contains(name))
            set(std::forward<type>(value));
    }

    auto set(const type &value) noexcept -> void {
        m_settings.setValue(m_name, value);
    }

    auto value() const noexcept -> type {
        return m_settings.value(m_name).value<type>();
    }

private:
    settings m_settings;
    const string m_name;
};

class viewer_settings {
public:
    viewer_settings() = default;

    settings_property<bool> view_word_wrap{"view/word_wrap", true};
    settings_property<bool> window_show_fullscreen{"window/fullscreen", false};
    settings_property<QRect> window_position{"window/position", {}};
};
