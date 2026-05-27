#pragma once
#include "vulkan/object_mgmt/resource/memory/interface/memory_pool.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace vulkan::object_mgmt {
    class Buffer {
        public:
            MemoryPool::Allocation allocation;
            vk::raii::Buffer buffer;

            Buffer(
                MemoryPool::Allocation allocation,
                vk::raii::Buffer buffer
            ) noexcept :
                allocation(std::move(allocation)),
                buffer(std::move(buffer))
            {};

            vk::raii::Buffer& operator * () noexcept {
                return buffer;
            }

            static std::expected<Buffer, Error> create(
                vk::raii::Device& device,
                MemoryPool& pool,
                const vk::BufferCreateInfo& info
            ) noexcept;
    };
}