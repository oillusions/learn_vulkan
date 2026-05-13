#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "physical_device.hpp"

namespace vulkan::context {
    class DeviceContext {
        public:
            PhysicalDeviceContext physical_device;
            vk::raii::Device device;
            std::vector<vk::raii::Queue> queues;

            DeviceContext(
                PhysicalDeviceContext physical_device,
                vk::raii::Device device,
                std::vector<vk::raii::Queue> queues
            ) noexcept :
                physical_device(std::move(physical_device)),
                device(std::move(device)),
                queues(std::move(queues))
            {}
    };
}