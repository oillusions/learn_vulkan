#pragma once

#include <error.hpp>

#include "vulkan/context/swap_chain.hpp"

namespace vulkan::object_mgmt::frame {
    class FrameManager {
        private:
            struct FrameState{
                vk::raii::CommandBuffer command_buffer;
                vk::raii::Fence sync_fence;
                vk::raii::Semaphore image_available_semaphore;
                vk::raii::Semaphore render_finished_semaphore;
            };

            vk::raii::Device& device;
            vulkan::context::SwapChainContext swap_chain_context;
            vk::raii::CommandPool command_pool;
            std::vector<FrameState> frame_states;

            FrameManager(
                vk::raii::Device& device,
                vulkan::context::SwapChainContext context,
                vk::raii::CommandPool command_pool,
                std::vector<FrameState> frame_states
            ) noexcept;

        public:
            static std::expected<FrameManager, Error> create(vk::raii::Device& device, vulkan::context::SwapChainContext context) noexcept;
        
    };
}