#include "application/impl/swap_chain.hpp"

namespace application::impl {

    inline std::expected<vk::raii::SwapchainKHR, Error> 
    create_swap_chain(
        vk::raii::Device& device
    ) noexcept {
        return Error("代码未完成");
    }

    std::expected<vulkan::context::SwapChainContext, Error> 
    create_swap_chain_context(
        vulkan::context::DeviceContext &device_context, 
        vk::raii::SurfaceKHR surface
    ) noexcept {

        auto result_swap_chain = create_swap_chain(device_context.device);
        if (!result_swap_chain) return result_swap_chain.error().forward("创建交换链失败");

        return Error("代码未完成");
    }
}