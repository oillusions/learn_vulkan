#pragma once

#include "vulkan/vulkan.hpp"
#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace vulkan::context {
    struct SwapChainConfig {
        vk::SurfaceFormatKHR surface_format;
        vk::PresentModeKHR present_mode;
        vk::DeviceSize image_count;
        vk::Extent2D image_extent;
        vk::CompositeAlphaFlagBitsKHR composite_alpha_flag_bits;  
    };

    class SwapChainContext {
        public:
            vk::raii::SurfaceKHR surface;
            vk::raii::SwapchainKHR swap_chain;
            std::vector<vk::Image> images;
            std::vector<vk::raii::ImageView> image_views;
            const SwapChainConfig config;

            SwapChainContext(
                vk::raii::SurfaceKHR surface,
                vk::raii::SwapchainKHR swap_chain,
                std::vector<vk::Image> images,
                std::vector<vk::raii::ImageView> image_views,
                const SwapChainConfig& config
            ) noexcept : 
                surface(std::move(surface)),
                swap_chain(std::move(swap_chain)),
                images(std::move(images)),
                image_views(std::move(image_views)),
                config(std::move(config))
            {};
    };
}