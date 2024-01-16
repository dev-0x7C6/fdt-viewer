#pragma once

#include <types.hpp>

#include <bit>
#include <cstring>

template <std::integral T>
constexpr T byteswap(T value) noexcept {
    static_assert(std::has_unique_object_representations_v<T>,
        "T may not have padding bits");
    auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
    std::ranges::reverse(value_representation);
    return std::bit_cast<T>(value_representation);
}

template <std::integral T>
T convert(const T data) noexcept {
    if constexpr (std::endian::native == std::endian::little)
        return byteswap(data);

    return data;
}

template <typename type, typename input_data>
constexpr type read_data_32be(input_data *input) noexcept {
    type container{};
    std::memcpy(reinterpret_cast<void *>(&container), reinterpret_cast<const void *>(input), sizeof(type));
    auto *data = reinterpret_cast<u32 *>(&container);
    for (auto i = 0; i < (sizeof(type) / sizeof(u32)); ++i)
        data[i] = convert<u32>(data[i]);
    return container;
}
