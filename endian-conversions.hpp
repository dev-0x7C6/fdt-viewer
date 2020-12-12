#pragma once

#include <bit>
#include <cstring>

constexpr u32 u32_be(const char *data) noexcept {
    if constexpr (std::endian::native == std::endian::big)
        return *reinterpret_cast<const u32 *>(data);

    if constexpr (std::endian::native == std::endian::little)
        return __builtin_bswap32(*reinterpret_cast<const u32 *>(data));

    return 0;
}

constexpr u32 u32_be(const u32 data) noexcept {
    if constexpr (std::endian::native == std::endian::big)
        return data;

    if constexpr (std::endian::native == std::endian::little)
        return __builtin_bswap32(data);

    return data;
}

template <typename type, typename input_data>
constexpr type read_data_32be(input_data *input) noexcept {
    type container{};
    std::memcpy(reinterpret_cast<void *>(&container), reinterpret_cast<const void *>(input), sizeof(type));
    auto *data = reinterpret_cast<u32 *>(&container);
    for (auto i = 0; i < (sizeof(type) / sizeof(u32)); ++i)
        data[i] = u32_be(data[i]);
    return container;
}
