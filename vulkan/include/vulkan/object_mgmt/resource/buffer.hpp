#pragma once

#include <cstring>
#include <vulkan/vulkan_raii.hpp>

#include "vulkan/object_mgmt/resource/memory/interface/memory_pool.hpp"

#include <error.hpp>
#include <utils/utils.hpp>

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



            template<typename ElementType>
            std::expected<void, Error> load_buffer_host(
                const vk::ArrayProxy<const ElementType>& array,
                const vk::DeviceSize size) {
                if (allocation.info.size >= size) {
                    auto& device_memory = allocation.memory_pool.getDeviceMemory();
                    auto result_memory_ptr = device_memory.mapMemory(allocation.info.offset, allocation.info.size)
                        | Error::from("映射内存失败");
                    if (!result_memory_ptr) return result_memory_ptr.error();
                    memcpy(result_memory_ptr.value(), array.data(), size);
                    device_memory.unmapMemory();
                }
                return {};
            }
        
            template<typename Container>
            std::expected<void, Error> load_buffer_host(
                const Container& container
                ) {
                using ElementType = typename Container::value_type;
                const auto array = vk::ArrayProxy<const ElementType>(container);
                
                return load_buffer_host(array, sizeof(ElementType) * array.size());
            }

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