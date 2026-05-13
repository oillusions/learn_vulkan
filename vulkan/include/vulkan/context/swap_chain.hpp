#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace vulkan::context {
    class SwapChainContext {
        public:
            vk::raii::SurfaceKHR surface;
            vk::raii::SwapchainKHR swap_chain;
            std::vector<vk::raii::Image> images;
            std::vector<vk::raii::ImageView> image_views;

            SwapChainContext(
                vk::raii::SurfaceKHR surface,
                vk::raii::SwapchainKHR swap_chain,
                std::vector<vk::raii::Image> images,
                std::vector<vk::raii::ImageView> image_views
            ) noexcept : 
                surface(std::move(surface)),
                swap_chain(std::move(swap_chain)),
                images(std::move(images)),
                image_views(std::move(image_views))
            {};
    };
}