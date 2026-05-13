#pragma once

#include <mutex>

#include <vulkan/vulkan_raii.hpp>

#include "error.hpp"
#include "interface/memory_pool.hpp"




namespace vulkan::utils {

    class LinearMemoryPool : public MemoryPool {
        private:
            vk::raii::DeviceMemory device_memory;
            vk::DeviceSize pool_size;
            vk::DeviceSize occupied;
            std::unique_ptr<std::mutex> mtx;
        public:
            explicit LinearMemoryPool(vk::raii::DeviceMemory device_memory, const vk::DeviceSize pool_size) noexcept :
                device_memory(std::move(device_memory)),
                pool_size(pool_size),
                occupied(0),
                mtx(std::make_unique<std::mutex>())
            {};

            LinearMemoryPool(LinearMemoryPool&& other) noexcept :
                device_memory(std::move(other.device_memory)),
                pool_size(other.pool_size),
                occupied(other.occupied),
            mtx(std::move(other.mtx))
            {};

            LinearMemoryPool& operator = (LinearMemoryPool&& other) noexcept {
                if (this != &other) {
                    this->device_memory = std::move(other.device_memory);
                    this->pool_size = other.pool_size;
                    this->occupied = other.occupied;
                    this->mtx = std::move(other.mtx);
                }
                return *this;
            }

            ~LinearMemoryPool() override = default;

            std::expected<Allocation, Error> allocate(const vk::DeviceSize size, const vk::DeviceSize alignment) noexcept override {
                auto lock = std::lock_guard(*mtx);
                const vk::DeviceSize alignment_offset = detail::alignUp(occupied, alignment);
                if (pool_size > alignment_offset) {
                    if ((pool_size - alignment_offset) >= size) {
                        occupied = alignment_offset + size;
                        return create_allocation(*this, detail::RangeInfo(alignment_offset, size));
                    }
                }
                return Error("剩余空间不足");
            }

            std::expected<Allocation, Error> allocate(const vk::MemoryRequirements &mem_req) noexcept override {
                return allocate(mem_req.size, mem_req.alignment);
            }

            vk::raii::DeviceMemory& getDeviceMemory() noexcept override {
                return device_memory;
            }

            void deallocate(Allocation &possessor) noexcept override {};

            void reset() noexcept override {
                auto lock = std::lock_guard(*mtx);
                occupied = 0;
            }
    };
}
