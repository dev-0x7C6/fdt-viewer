#pragma once

#include <types.hpp>

constexpr auto FDT_MAGIC_VALUE = 0xD00DFEED;

enum class FDT_TOKEN : u32 {
    BEGIN_NODE = 0x00000001,
    END_NODE = 0x00000002,
    PROPERTY = 0x00000003,
    NOP = 0x00000004,
    END = 0x00000009,
};

constexpr auto name(const FDT_TOKEN token) -> const char * {
    switch (token) {
        case FDT_TOKEN::BEGIN_NODE: return "begin_node";
        case FDT_TOKEN::END_NODE: return "end_node";
        case FDT_TOKEN::PROPERTY: return "property";
        case FDT_TOKEN::NOP: return "nop";
        case FDT_TOKEN::END: return "end";
    }

    return nullptr;
}

struct fdt_header {
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

struct fdt_property_header {
    u32 len;
    u32 nameoff;
};

static_assert(sizeof(fdt_header) == 40);
static_assert(sizeof(fdt_property_header) == 8);
