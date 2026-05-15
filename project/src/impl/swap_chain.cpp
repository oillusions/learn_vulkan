#include "application/impl/swap_chain.hpp"


namespace application::impl {

    std::expected<vulkan::context::SwapChainContext, Error> 
    create_swap_chain_context(
        vulkan::context::DeviceContext &device_context, 
        vk::raii::SurfaceKHR surface,
        const SwapChainConfig& config
    ) noexcept {

        auto result_swap_chain_config = vulkan::context::SwapChainContext::obtain_config(
            device_context.physical_device.physical_device,
            surface
        );
        if (!result_swap_chain_config) return result_swap_chain_config.error().forward("获取交换链配置失败");
        const auto swap_chain_config = std::move(result_swap_chain_config).value();
    
        
        return vulkan::context::SwapChainContext::create(
            device_context.device,
            std::move(surface), 
            swap_chain_config
        );
    }
}