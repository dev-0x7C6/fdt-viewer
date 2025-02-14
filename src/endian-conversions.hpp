#pragma once

#include <algorithm>
#include <types.hpp>

#include <bit>
#include <span>
#include <cstring>

template <std::integral integral>
constexpr auto byteorder(const integral data) noexcept -> integral {
    if constexpr (std::endian::native == std::endian::little)
        return std::byteswap(data);
    return data;
}

namespace modify {
template <std::integral type>
constexpr auto byteorder(type &data) noexcept -> void {
    data = ::byteorder(data);
}
} // namespace modify

template <typename type, typename input_data>
constexpr type read_data_32be(const input_data *input) noexcept {
    type container{};
    std::memcpy(reinterpret_cast<void *>(&container), reinterpret_cast<const void *>(input), sizeof(type));
    std::span<u32> data(reinterpret_cast<u32 *>(&container), sizeof(type) / 4);
    std::ranges::for_each(data, modify::byteorder<u32>);
    return container;
}
