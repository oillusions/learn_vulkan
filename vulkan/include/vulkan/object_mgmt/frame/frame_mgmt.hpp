#pragma once

#include <error.hpp>
#include <utility>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "vulkan/context/swap_chain.hpp"
#include "vulkan/object_mgmt/render_pass.hpp"

namespace vulkan::object_mgmt::frame {
    class FrameManager {
        private:
            struct FrameState{
                vk::raii::CommandBuffer command_buffer;
                vk::raii::Fence sync_fence;
                vk::raii::Semaphore image_available_semaphore;
                vk::raii::Semaphore render_finished_semaphore;
            };

            class Token {
                private:
                    bool is_release{false};
                public:
                    FrameManager& manager;
                    vk::raii::CommandBuffer& command_buffer;
                    vk::raii::Framebuffer& frame_buffer;
                    const uint32_t image_index;
                    const uint32_t frame_sync_index;

                    Token(
                        FrameManager& manager,
                        vk::raii::CommandBuffer& command_buffer,
                        vk::raii::Framebuffer& frame_buffer,
                        const uint32_t image_index,
                        const uint32_t frame_sync_index
                    ) noexcept :
                        manager(manager),
                        command_buffer(command_buffer),
                        frame_buffer(frame_buffer),
                        image_index(image_index),
                        frame_sync_index(frame_sync_index)
                    {};

                    void present() {
                        if (!is_release) {
                            manager.present(*this);
                            is_release = true;
                        }
                    }
                    
                    Token(Token&& other) noexcept :
                        manager(other.manager),
                        command_buffer(other.command_buffer),
                        frame_buffer(other.frame_buffer),
                        image_index(other.image_index),
                        frame_sync_index(other.frame_sync_index),
                        is_release(std::exchange(other.is_release, true)) 
                    {};
                    
                    Token(const Token&) = delete;
                    Token& operator = (const Token&) = delete;
            };
            friend Token;

            const vk::raii::PhysicalDevice& physical_device;
            vk::raii::Device& device;
            vk::raii::Queue& queue;
            vulkan::context::SwapChainContext swap_chain_context;

            vk::raii::CommandPool command_pool;
            std::vector<FrameState> frame_states;
            std::vector<vk::raii::Framebuffer> frame_buffers;

            vk::DeviceSize curr_frame_sync;

            FrameManager(
                const vk::raii::PhysicalDevice& physical_device,
                vk::raii::Device& device,
                vk::raii::Queue& queue,
                vulkan::context::SwapChainContext context,
                vk::raii::CommandPool command_pool,
                std::vector<FrameState> frame_states,
                std::vector<vk::raii::Framebuffer> frame_buffers
            ) noexcept;

            void present(Token& token);
        public:
            FrameManager(FrameManager&& other) noexcept :
                physical_device(other.physical_device),
                device(other.device),
                queue(other.queue),
                swap_chain_context(std::move(other.swap_chain_context)),
                command_pool(std::move(other.command_pool)),
                frame_states(std::move(other.frame_states)),
                frame_buffers(std::move(other.frame_buffers)),
                curr_frame_sync(other.curr_frame_sync)
            {};
            FrameManager(const FrameManager&) = delete;
            FrameManager& operator = (FrameManager&&) noexcept = delete;
            FrameManager& operator = (const FrameManager&) = delete;
            ~FrameManager() = default;

            std::expected<void, Error> rebuild(RenderPass& pass) noexcept;

            std::optional<std::expected<FrameManager::Token, Error>> obtain_frame_command_buffer() noexcept;

            vk::Extent2D obtain_frame_size() noexcept {
                return swap_chain_context.config.image_extent;
            }

            static std::expected<FrameManager, Error> create(
                const vk::raii::PhysicalDevice& physical_device,
                vk::raii::Device& device, 
                vk::raii::Queue& queue, 
                vulkan::context::SwapChainContext context,
                RenderPass& pass
            ) noexcept;
        
    };
}