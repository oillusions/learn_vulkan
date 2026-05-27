#include "application/impl/mem.hpp"

namespace application::impl {
    std::expected<vk::DeviceSize, Error> find_mem_type_inc(
        const vk::raii::PhysicalDevice& physical_device,
        const uint32_t type_bits,
        const vk::MemoryPropertyFlags& requires_props
        ) noexcept {
        const auto memory_properties = physical_device.getMemoryProperties();

        size_t index{0};
        for (const auto& type : memory_properties.memoryTypes) {
            if ((type_bits & (1 << index)) && ((type.propertyFlags & requires_props) == requires_props)) {
                return static_cast<vk::DeviceSize>(index);
            }
            index++;
        }
        return Error("未找到需要的内存类型");
    }

    std::expected<vk::DeviceSize, Error> find_type_same(
        const vk::raii::PhysicalDevice& physical_device, 
        const vk::MemoryPropertyFlags flags
    ) noexcept {
        const auto memory_properties = physical_device.getMemoryProperties();
        size_t index{0};
        for (const auto& type : memory_properties.memoryTypes) {
            if (type.propertyFlags == flags) {
                return static_cast<vk::DeviceSize>(index);
            }
            index++;
        }
        return Error("未找到需要的内存类型");
    }
}