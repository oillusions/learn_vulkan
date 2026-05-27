#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <error.hpp>
#include "vulkan/object_mgmt/resource/memory/detail.hpp"


namespace vulkan::object_mgmt {
    class MemoryPool {
        public:
            class Allocation {
                public:
                    friend MemoryPool;
                    MemoryPool& memory_pool;
                    const detail::RangeInfo info;

                protected:
                    bool released{false};

                    Allocation(MemoryPool& memory_pool, const detail::RangeInfo info) noexcept :
                            memory_pool(memory_pool),
                            info(info),
                            released(false)
                    {}

                public:

                    Allocation(Allocation&& other) noexcept :
                        memory_pool(other.memory_pool),
                        info(other.info),
                        released(other.released) {
                        other.released = true;
                    }

                    Allocation& operator = (Allocation&&) noexcept = delete;

                    Allocation(const Allocation&) = delete;
                    Allocation& operator = (const Allocation&) = delete;

                    void deallocate() noexcept {
                        if (!released) {
                            memory_pool.deallocate(*this);
                            released = true;
                        }
                    }

                    ~Allocation() {
                        deallocate();
                    }
            };
        public:
            MemoryPool() = default;

            MemoryPool(const MemoryPool&) = delete;
            MemoryPool& operator = (const MemoryPool&) = delete;

            virtual std::expected<Allocation, Error> allocate(const vk::DeviceSize size, const vk::DeviceSize alignment) noexcept = 0;
            virtual std::expected<Allocation, Error> allocate(const vk::MemoryRequirements& mem_req) noexcept {
                return allocate(mem_req.size, mem_req.alignment);
            }

            virtual vk::raii::DeviceMemory& getDeviceMemory() noexcept = 0;

            virtual void deallocate(Allocation& possessor) noexcept = 0;

            virtual void reset() noexcept {};

            virtual ~MemoryPool() = default;

        protected:

            static Allocation create_allocation(MemoryPool& memory_pool, const detail::RangeInfo range_info) noexcept {
                return Allocation(memory_pool, range_info);
            }

            static bool get_allocation_released_flag(const Allocation& allocation) noexcept {
                return allocation.released;
            }

            static void set_allocation_released_flag(Allocation& possessor) noexcept {
                possessor.released = true;
            }
    };
}
