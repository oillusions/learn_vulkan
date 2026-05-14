#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <error.hpp>

#include "vulkan/context/device.hpp"
#include "vulkan/context/swap_chain.hpp"


namespace application::impl {
    struct SwapChainConfig {
        
    };

    std::expected<vulkan::context::SwapChainContext, Error> 
    create_swap_chain_context(
        vulkan::context::DeviceContext& device_context,
        vk::raii::SurfaceKHR surface,
        const SwapChainConfig& config
    ) noexcept;
}