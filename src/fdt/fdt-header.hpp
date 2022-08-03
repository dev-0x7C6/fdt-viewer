#pragma once

#include <types.hpp>

constexpr auto FDT_MAGIC_VALUE = 0xD00DFEED;
constexpr auto FDT_SUPPORT_ABOVE = 16;

namespace fdt {

enum class token : u32 {
    begin_node = 0x01,
    end_node = 0x02,
    property = 0x03,
    nop = 0x04,
    end = 0x09,
};

constexpr auto name(const token value) noexcept -> const char * {
    switch (value) {
        case token::begin_node: return "begin_node";
        case token::end_node: return "end_node";
        case token::property: return "property";
        case token::nop: return "nop";
        case token::end: return "end";
    }

    return nullptr;
}

struct header {
    u32 magic;
    u32 totalsize;
    u32 off_dt_struct;
    u32 off_dt_strings;
    u32 off_mem_rsvmap;
    u32 version;
    u32 last_comp_version;
    u32 boot_cpuid_phys;
    u32 size_dt_strings;
    u32 size_dt_struct;
};

struct property {
    u32 len;
    u32 nameoff;
};

static_assert(sizeof(header) == 40);
static_assert(sizeof(property) == 8);
}; // namespace fdt
