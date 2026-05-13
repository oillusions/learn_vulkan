#pragma once

#include <vulkan/vulkan.hpp>

namespace vulkan::detail {
    enum class FreeListAllocatorStrategy {
        FirstFit = 0,
        BestFit,
    };

    struct RangeInfo {
        vk::DeviceSize offset;
        vk::DeviceSize size;

        RangeInfo(const vk::DeviceSize offset, const vk::DeviceSize size) noexcept :
            offset(offset),
            size(size)
        {};

        bool operator == (const RangeInfo& other) const {
            return offset == other.offset && size == other.size;
        }
    };

    inline vk::DeviceSize alignUp(const vk::DeviceSize value, const vk::DeviceSize alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }
}