#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "vulkan/utils/queue_family.hpp"
#include <vector>

namespace vulkan::context {
    class PhysicalDeviceContext {
        public:
            vk::raii::PhysicalDevice physical_device;
            const std::vector<vulkan::utils::QueueFamily> queue_families;
            
            PhysicalDeviceContext(
                vk::raii::PhysicalDevice physical_device, 
                std::vector<vulkan::utils::QueueFamily> queue_families
            ) noexcept :
                physical_device(std::move(physical_device)),
                queue_families(std::move(queue_families))
            {}
    };
}