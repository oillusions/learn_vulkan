#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <error.hpp>

#include <utils/utils.hpp>

#include "vulkan/context/device.hpp"
#include "vulkan/object_mgmt/resource/memory/interface/memory_pool.hpp"


namespace application::impl {
    std::expected<vk::DeviceSize, Error> find_mem_type_inc(
        const vk::raii::PhysicalDevice& physical_device,
        const uint32_t type_bits,
        const vk::MemoryPropertyFlags& requires_props
    ) noexcept;

    std::expected<vk::DeviceSize, Error> find_type_same(
        const vk::raii::PhysicalDevice& physical_device, 
        const vk::MemoryPropertyFlags flags
    ) noexcept;

    template<typename PoolType>
    requires std::derived_from<PoolType, vulkan::object_mgmt::MemoryPool>
    std::expected<PoolType, Error> create_memory_pool(
        vulkan::context::DeviceContext &device_context,
        const vk::MemoryPropertyFlags flag,
        const vk::DeviceSize size
    ) noexcept {
        const auto result_mem_type_index = find_type_same(
            device_context.physical_device.physical_device, 
            flag
        );
        if (!result_mem_type_index) return result_mem_type_index.error().forward("查找内存类型索引失败");
        const auto mem_type_index = result_mem_type_index.value();

        const auto allocate_info = vk::MemoryAllocateInfo()
                .setAllocationSize(size)
                .setMemoryTypeIndex(mem_type_index);

        auto result_device_mem = device_context.device.allocateMemory(allocate_info)
            | Error::from("内存对象创建失败");
        if (!result_device_mem) return result_device_mem.error();

        return PoolType(std::move(result_device_mem.value()), size);
    }
}