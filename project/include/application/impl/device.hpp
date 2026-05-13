#pragma once

#include <set>

#include <vulkan/vulkan_raii.hpp>

#include <Error.hpp>

#include "vulkan/context/instace.hpp"
#include "vulkan/context/device.hpp"



namespace application::impl {
    namespace device {
        std::set<const char*> obtain_essential_extensions() noexcept;
    }

    std::expected<vulkan::context::DeviceContext, Error> create_device_context(vulkan::context::InstanceContext& instance_context) noexcept;

}