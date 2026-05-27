#pragma once
#include <list>
#include <mutex>

#include <vulkan/vulkan_raii.hpp>

#include <error.hpp>

#include "detail.hpp"
#include "vulkan/object_mgmt/resource/memory/interface/memory_pool.hpp"

namespace vulkan::object_mgmt {
    template<detail::AllocatorStrategy Strategy>
    class FreeListMemoryPool : public MemoryPool {
        public:
            vk::raii::DeviceMemory device_memory;
            vk::DeviceSize pool_size;

        private:
            std::list<detail::RangeInfo> free_list;
            std::unique_ptr<std::mutex> mtx;

        public:
            explicit FreeListMemoryPool(vk::raii::DeviceMemory device_memory, const vk::DeviceSize pool_size) noexcept :
                device_memory(std::move(device_memory)),
                pool_size(pool_size),
                mtx(std::make_unique<std::mutex>()){
                free_list.emplace_back(detail::RangeInfo{0, pool_size});
            };

            FreeListMemoryPool(const FreeListMemoryPool&) = delete;
            FreeListMemoryPool& operator = (const FreeListMemoryPool&) = delete;

            FreeListMemoryPool(FreeListMemoryPool&& other) noexcept :
                device_memory(std::move(other.device_memory)),
                pool_size(other.pool_size),
                free_list(std::move(other.free_list)),
                mtx(std::move(other.mtx))
            {};
            FreeListMemoryPool& operator = (FreeListMemoryPool&& other) noexcept {
                if (this != &other) {
                    this->device_memory = std::move(other.device_memory);
                    this->pool_size = other.pool_size;
                    this->free_list = std::move(other.free_list);
                    this->mtx = std::move(other.mtx);
                }
                return *this;
            }

            ~FreeListMemoryPool() override = default;

            std::expected<Allocation, Error> allocate(const vk::DeviceSize size, const vk::DeviceSize alignment) noexcept override {
                auto lock = std::lock_guard(*mtx);
                if constexpr (Strategy == detail::AllocatorStrategy::FirstFit) {
                    for (auto it = free_list.begin(); it != free_list.end(); ++it) {
                        const auto aligned_offset = detail::alignUp(it->offset, alignment);
                        const auto waste_size = aligned_offset - it->offset;
                        const auto available_size = it->size - waste_size;

                        if (available_size >= size) {
                            if (waste_size > 0) {
                                free_list.insert(it, detail::RangeInfo(it->offset, waste_size));

                                it->offset = aligned_offset;
                                it->size = available_size;
                            }
                            if (available_size == size) {
                                free_list.erase(it);
                                return create_allocation(*this, detail::RangeInfo(aligned_offset, size));
                            } else {
                                const auto remaining_size = available_size - size;

                                if (remaining_size >= 32) {
                                    it->offset = aligned_offset + size;
                                    it->size = remaining_size;
                                    return create_allocation(*this, detail::RangeInfo(aligned_offset, size));
                                }

                                free_list.erase(it);
                                return create_allocation(*this, detail::RangeInfo(aligned_offset, size + remaining_size));
                            }
                        }
                    };
                    return Error("未找到合适的空闲空间");
                } else if constexpr (Strategy == detail::AllocatorStrategy::BestFit) {
                    auto best_it = free_list.end();
                    vk::DeviceSize best_available_size = 0;
                    for (auto it = free_list.begin(); it != free_list.end(); ++it) {
                        const auto aligned_offset = detail::alignUp(it->offset, alignment);
                        const auto waste_size = aligned_offset - it->offset;
                        const auto available_size = it->size - waste_size;

                        if (available_size >= size) {
                            if (best_it == free_list.end() || available_size < best_available_size) {
                                best_it = it;
                                best_available_size = available_size;
                            }
                        }
                    }
                    if (best_it != free_list.end()) {
                        const auto aligned_offset = detail::alignUp(best_it->offset, alignment);
                        const auto waste_size = aligned_offset - best_it->offset;
                        const auto available_size = best_it->size - waste_size;

                        if (waste_size > 0) {
                            free_list.insert(best_it, detail::RangeInfo(best_it->offset, waste_size));

                            best_it->offset = aligned_offset;
                            best_it->size = available_size;
                        }
                        if (available_size == size) {
                            free_list.erase(best_it);
                            return create_allocation(*this, detail::RangeInfo(aligned_offset, size));
                        } else {
                            const auto remaining_size = available_size - size;

                            if (remaining_size >= 32) {
                                best_it->offset = aligned_offset + size;
                                best_it->size = remaining_size;
                                return create_allocation(*this, detail::RangeInfo(aligned_offset, size));
                            }

                            free_list.erase(best_it);
                            return create_allocation(*this, detail::RangeInfo(aligned_offset, size + remaining_size));
                        }
                    }

                    return Error("未找到合适的空闲空间");
                }
                return Error("未知分配器策略");
            }

            auto allocate(const vk::MemoryRequirements &mem_req) noexcept -> decltype(allocate(mem_req.size, mem_req.alignment)) override {
                return allocate(mem_req.size, mem_req.alignment);
            }

        private:
            void space_merging(const detail::RangeInfo& range) {
                auto it = free_list.begin();
                while (it != free_list.end() && it->offset < range.offset) {
                    ++it;
                };

                it = free_list.insert(it, range);

                auto forward_merge_it = free_list.end();
                while (it != free_list.begin()) {
                    forward_merge_it = std::prev(it);
                    if (forward_merge_it->offset + forward_merge_it->size == it->offset) {
                        it->offset = forward_merge_it->offset;
                        it->size += forward_merge_it->size;
                        free_list.erase(forward_merge_it);
                    } else {
                        break;
                    }
                }

                auto backward_merge_it = free_list.end();
                while (it != free_list.end()) {
                    backward_merge_it = std::next(it);
                    if (backward_merge_it == free_list.end()) break;
                    if (it->offset + it->size == backward_merge_it->offset) {
                        it->size += backward_merge_it->size;
                        free_list.erase(backward_merge_it);
                    } else {
                        break;
                    }
                }
            }

        public:
            void deallocate(Allocation& allocation) noexcept override {
                auto lock = std::lock_guard(*mtx);
                if (get_allocation_released_flag(allocation)) return;
                set_allocation_released_flag(allocation);

                space_merging(allocation.info);
            }

            vk::raii::DeviceMemory& getDeviceMemory() noexcept override {
                return device_memory;
            }

            const std::list<detail::RangeInfo>& get_free_list() const noexcept {
                return free_list;
            }
        };
}
