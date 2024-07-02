#pragma once

#include <types.hpp>

namespace fdt {

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

constexpr auto is_magic_invalid(const header &v) -> bool {
    constexpr auto header_magic_value = 0xD00DFEED;
    return v.magic != header_magic_value;
}

constexpr auto is_version_unsupported(const header &v) -> bool {
    constexpr auto header_support_above = 16;
    return v.version <= header_support_above;
}

static_assert(sizeof(header) == 40);
static_assert(sizeof(property) == 8);
}; // namespace fdt
