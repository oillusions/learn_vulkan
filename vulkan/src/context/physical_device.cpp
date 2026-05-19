#include "vulkan/context/physical_device.hpp"



namespace vulkan::context {
    PhysicalDeviceContext
    PhysicalDeviceContext::create(
        vk::raii::PhysicalDevice physical_device
    ) noexcept {
        const auto queue_family_properties = physical_device.getQueueFamilyProperties();

        auto queue_family_counters = std::vector<vulkan::utils::QueueFamily>();
        queue_family_counters.reserve(queue_family_properties.size());

        vk::DeviceSize index{0}; 
        for (const auto& properters : queue_family_properties) {
            queue_family_counters.emplace_back(properters, index);
            index++;
        }

        return vulkan::context::PhysicalDeviceContext(
            std::move(physical_device),
            std::move(queue_family_counters)
        );
    }
}