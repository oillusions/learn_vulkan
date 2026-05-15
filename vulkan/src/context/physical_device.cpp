#include "vulkan/context/physical_device.hpp"
#include <ranges>

namespace vulkan::context {
    PhysicalDeviceContext
    PhysicalDeviceContext::create(
        vk::raii::PhysicalDevice physical_device
    ) noexcept {
        const auto queue_family_properties = physical_device.getQueueFamilyProperties();

        auto queue_family_counters = std::vector<vulkan::utils::QueueFamily>();
        queue_family_counters.reserve(queue_family_properties.size());
        for (const auto& [index, properters] : queue_family_properties | std::views::enumerate) {
            queue_family_counters.emplace_back(properters, index);
        }

        return vulkan::context::PhysicalDeviceContext(
            std::move(physical_device),
            std::move(queue_family_counters)
        );
    }
}