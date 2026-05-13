#include "vulkan/utils/queue_family.hpp"

namespace vulkan::utils {
    void QueueFamily::Possessor::deallocate() noexcept {
        if (!is_release) {
            queue_family.deallocate(*this);
            is_release = true;
        }
    }

    QueueFamily::Possessor::~Possessor() noexcept {
        deallocate();
    }

    vk::DeviceSize QueueFamily::queueRemainCount() const noexcept {
        if (queue_occupancy_count <= queue_count) {
            return queue_count - queue_occupancy_count;
        }
        return 0;
    }

    std::expected<QueueFamily::Possessor, Error> QueueFamily::allocate(const vk::DeviceSize count) noexcept {
        if (queueRemainCount() >= count) {
            queue_occupancy_count += count;
            return Possessor(*this, count);
        }
        return Error("队列剩余数量不足以分配");
    }

    void QueueFamily::deallocate(QueueFamily::Possessor& possessor) noexcept {
        if (!possessor.is_release) {
            if (possessor.queue_occupancy_count + queueRemainCount() <= queue_count) {
                queue_occupancy_count -= possessor.queue_occupancy_count;
                possessor.is_release = true;
            }
        }
    }
}