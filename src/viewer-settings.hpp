#pragma once

#include <QSettings>
#include <QVariant>

#include <types.hpp>

using settings = QSettings;

class settings_group_raii {
public:
    settings_group_raii(const string &group) {
        ref.beginGroup(group);
    }

    ~settings_group_raii() {
        ref.endGroup();
    }

    settings *operator->() {
        return &ref;
    }

private:
    settings ref;
};

template <typename type>
class settings_property {
public:
    settings_property(const string &group, const string &name, type &&value = {})
            : m_group(group)
            , m_name(name) {
        set(std::move(value));
    }

    auto set(const type &value) noexcept -> void {
		settings_group_raii(m_group)->setValue(m_name, value);
	}

	auto value() const noexcept -> type {
		return settings_group_raii(m_group)->value(m_name).value<type>();
	}

private:
    const string m_group;
    const string m_name;
};

class viewer_settings {
public:
    viewer_settings() = default;

    settings_property<bool> view_word_wrap{"view", "word_wrap", true};
    settings_property<bool> window_show_fullscreen{"window", "fullscreen", false};
};
