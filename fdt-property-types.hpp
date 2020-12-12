#pragma once

#include <string>
#include <unordered_map>

enum class property_type {
    hex,
    number,
    string,
    multiline,
    guess,
};

enum class word_size {
    _8,
    _16,
    _32,
    _64,
    custom,
};

struct property_info {
    property_type type{property_type::guess};
    word_size word{word_size::_8};
};

const static std::unordered_map<std::string, property_info> = {
    {"#address-cells", {property_type::number, word_size::_32}},
    {"#size-cells", {property_type::number, word_size::_32}},
    {"compatible", {property_type::string, word_size::custom}},
    {"phandle", {property_type::number, word_size::_32}},
    {"pinctrl-names", {property_type::multiline, word_size::custom}},
}
