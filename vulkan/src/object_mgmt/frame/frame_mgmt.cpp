#include "vulkan/object_mgmt/frame/frame_mgmt.hpp"
#include "vulkan/context/swap_chain.hpp"
#include "vulkan/object_mgmt/render_pass.hpp"
#include "vulkan/vulkan.hpp"

#include <cstdint>
#include <error.hpp>
#include <optional>
#include <utils/utils.hpp>


static constexpr uint64_t timeout = 1000000000; 

namespace vulkan::object_mgmt::frame {
    FrameManager::FrameManager(
        const vk::raii::PhysicalDevice& physical_device,
        vk::raii::Device& device,
        vk::raii::Queue& queue,
        vulkan::context::SwapChainContext context,
        vk::raii::CommandPool command_pool,
        std::vector<FrameState> frame_states,
        std::vector<vk::raii::Framebuffer> frame_buffers
        ) noexcept :
            physical_device(physical_device),
            device(device),
            queue(queue),
            swap_chain_context(std::move(context)),
            command_pool(std::move(command_pool)),
            frame_states(std::move(frame_states)),
            frame_buffers(std::move(frame_buffers)),
            curr_frame_sync(0)
    {}

    std::expected<void, Error> FrameManager::rebuild(RenderPass& pass) noexcept {

        const auto result_config = context::SwapChainContext::obtain_config(
            physical_device, 
            swap_chain_context.surface
        );
        if (!result_config) return result_config.error().forward("获取交换链配置失败");

        device.waitIdle();
        
        auto result_new_swap_chain = context::SwapChainContext::create(
            device, 
            std::move(swap_chain_context.surface),
            result_config.value(),
            std::move(swap_chain_context)
        );
        if (!result_new_swap_chain) return result_new_swap_chain.error().forward("重建交换链失败");

        auto result_new_frame_mgmt = FrameManager::create(
            physical_device, device, 
            queue, 
            std::move(result_new_swap_chain.value()), 
            pass
        );
        if (!result_new_frame_mgmt) return result_new_frame_mgmt.error().forward("重建帧管理器失败");
        auto self = this;

        this->~FrameManager();

        new (this) FrameManager(std::move(result_new_frame_mgmt.value()));

        return {};
    }

    std::optional<std::expected<FrameManager::Token, Error>> FrameManager::obtain_frame_command_buffer() noexcept {
        auto& sync = frame_states[curr_frame_sync];
        
        auto result = device.waitForFences(*sync.sync_fence, vk::True, timeout);
        if (result != vk::Result::eSuccess) {
            return Error("等待同步栅栏失败", vk::to_string(result));
        }
        device.resetFences(*sync.sync_fence);

        auto result_image_index = swap_chain_context.swap_chain.acquireNextImage(
            timeout, 
            sync.image_available_semaphore
        )   | Error::from("获取交换链图像索引失败");
        if (!result_image_index) {
            return std::nullopt;
        }
        const auto image_index = result_image_index.value();
        
        return Token(
            *this,
            frame_states[curr_frame_sync].command_buffer,
            frame_buffers[image_index],
            image_index,
            curr_frame_sync
        );
    }

    void FrameManager::present(Token& token) {
        auto& sync = frame_states[token.frame_sync_index];

        vk::Result result;

        const vk::PipelineStageFlags wait_dst_stage_mask[] = {
            vk::PipelineStageFlagBits::eColorAttachmentOutput
        };

        const auto submit_info = vk::SubmitInfo()
            .setCommandBuffers(*sync.command_buffer)
            .setWaitSemaphores(*sync.image_available_semaphore)
            .setSignalSemaphores(*sync.render_finished_semaphore)
            .setWaitDstStageMask(wait_dst_stage_mask);
        result = queue.submit(submit_info, sync.sync_fence);
        if (result != vk::Result::eSuccess) {
            throw Error("命令缓冲区提交异常", vk::to_string(result));
        }

        const auto present_info = vk::PresentInfoKHR()
            .setWaitSemaphores(*sync.render_finished_semaphore)
            .setSwapchains(**swap_chain_context)
            .setImageIndices(token.image_index);

        result = queue.presentKHR(present_info);
        if (result != vk::Result::eSuccess) {
            throw Error("呈现异常", vk::to_string(result));
        }
        
        curr_frame_sync = (curr_frame_sync + 1) % frame_states.size();
    }

    std::expected<FrameManager, Error> FrameManager::create(
        const vk::raii::PhysicalDevice& physical_device,
        vk::raii::Device& device, 
        vk::raii::Queue& queue,
        vulkan::context::SwapChainContext context,
        RenderPass& pass
    ) noexcept {
        const auto frame_state_count = context.images.size() + 1;

        std::vector<FrameManager::FrameState> frame_states;
        frame_states.reserve(frame_state_count);

        const auto command_pool_info = vk::CommandPoolCreateInfo()
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(0);
        auto result_command_pool = device.createCommandPool(command_pool_info)
            | Error::from("创建指令池失败");
        auto command_pool = std::move(result_command_pool).value();

        const auto command_buffer_allocate_info = vk::CommandBufferAllocateInfo()
            .setCommandPool(command_pool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(frame_state_count);

        auto result_command_buffers = device.allocateCommandBuffers(command_buffer_allocate_info)
            | Error::from("创建指令缓冲区失败");
        if (!result_command_buffers) return result_command_buffers.error();
        auto command_buffers = std::move(result_command_buffers).value();
    
        const auto fence_info = vk::FenceCreateInfo()
            .setFlags(vk::FenceCreateFlagBits::eSignaled);

        const auto semaphore_info = vk::SemaphoreCreateInfo();

        for (vk::DeviceSize i = 0; i < frame_state_count; i++) {
            auto result_fence = device.createFence(fence_info)
                | Error::from("创建同步栅栏失败");
            if (!result_fence) return result_fence.error();

            auto result_semaphore_0 = device.createSemaphore(semaphore_info)
                | Error::from("创建信号量失败");
            if (!result_semaphore_0) return result_semaphore_0.error();

            auto result_semaphore_1 = device.createSemaphore(semaphore_info)
                | Error::from("创建信号量失败");
            if (!result_semaphore_1) return result_semaphore_1.error();

            frame_states.emplace_back(
                std::move(command_buffers[i]),
                std::move(result_fence).value(),
                std::move(result_semaphore_0).value(),
                std::move(result_semaphore_1).value()
            );
        }

        auto frame_buffer_info = vk::FramebufferCreateInfo()
            .setRenderPass(*pass)
            .setAttachmentCount(1)
            .setLayers(1)
            .setWidth(context.config.image_extent.width)
            .setHeight(context.config.image_extent.height);
        
        std::vector<vk::raii::Framebuffer> frame_buffers;
        frame_buffers.reserve(context.image_views.size());
        for (const auto& view : context.image_views) {
            frame_buffer_info.setAttachments(*view);
            auto result_frame_buffer = device.createFramebuffer(frame_buffer_info)
                | Error::from("帧缓冲对象创建失败");
            
            frame_buffers.emplace_back(std::move(result_frame_buffer.value()));
        }

        return FrameManager(
            physical_device,
            device,
            queue,
            std::move(context),
            std::move(command_pool),
            std::move(frame_states),
            std::move(frame_buffers)
        );
    }
}