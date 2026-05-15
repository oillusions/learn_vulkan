#include "vulkan/object_mgmt/frame/frame_mgmt.hpp"

#include <error.hpp>
#include "utils/utils.hpp"
#include "vulkan/vulkan.hpp"

static constexpr uint64_t timeout = 1000000; 

namespace vulkan::object_mgmt::frame {
    FrameManager::FrameManager(
        const vk::raii::PhysicalDevice& physical_device,
        vk::raii::Device& device,
        vk::raii::Queue& queue,
        vulkan::context::SwapChainContext context,
        vk::raii::CommandPool command_pool,
        std::vector<FrameState> frame_states
        ) noexcept :
            physical_device(physical_device),
            device(device),
            queue(queue),
            swap_chain_context(std::move(context)),
            command_pool(std::move(command_pool)),
            frame_states(std::move(frame_states))
    {}

    inline vk::Extent2D obtain_image_extent(
        const vk::SurfaceCapabilitiesKHR& surface_capabilities
    ) noexcept {
        const auto& min_image_extent = surface_capabilities.minImageExtent;
        const auto& max_image_extent = surface_capabilities.maxImageExtent;
        const auto& curr_image_extent = surface_capabilities.currentExtent;

        const auto available_width = std::clamp(
            curr_image_extent.width, min_image_extent.width, max_image_extent.width
        );
        const auto available_height = std::clamp(
            curr_image_extent.height, min_image_extent.height, max_image_extent.height
        );

        return vk::Extent2D(available_width, available_height);
    }

    std::expected<void, Error> FrameManager::rebuild_swap_chain() noexcept {
        auto config = swap_chain_context.config;
        auto result_surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(swap_chain_context.surface)
            | Error::from("获取表面支持的能力信息失败");
        config.image_extent = obtain_image_extent(result_surface_capabilities.value());

        const auto swap_chain_info = vk::SwapchainCreateInfoKHR()
            .setSurface(swap_chain_context.surface)
            .setPresentMode(config.present_mode)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageFormat(config.surface_format.format)
            .setImageColorSpace(config.surface_format.colorSpace)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setImageArrayLayers(1)
            .setMinImageCount(config.image_count)
            .setImageExtent(config.image_extent)
            .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
            .setClipped(vk::True)
            .setCompositeAlpha(config.composite_alpha_flag_bits)
            .setOldSwapchain(swap_chain_context.swap_chain);

        auto result_swap_chain = device.createSwapchainKHR(swap_chain_info)
            | Error::from("创建交换链对象失败");
            swap_chain_context.swap_chain = std::move(result_swap_chain).value();

        return {};
    }

    std::expected<FrameManager::Token, Error> FrameManager::obtain_frame_command_buffer() noexcept {
        auto& frame_state = frame_states[frame_state_index];
        
        auto result = device.waitForFences(*frame_state.sync_fence, vk::True, timeout);
        if (result != vk::Result::eSuccess) {
            return Error("等待同步栅栏失败");
        }
        device.resetFences(*frame_state.sync_fence);

        auto result_image_index = swap_chain_context.swap_chain.acquireNextImage(
            timeout, 
            frame_state.image_available_semaphore, 
            nullptr
        )   | Error::from("获取交换链图像索引失败");
        if (!result_image_index) {
            auto result = rebuild_swap_chain();
            if (!result) return result.error().forward("重建交换链失败");
            return obtain_frame_command_buffer();
        }
        const auto image_index = result_image_index.value();

        return Token(
            *this,
            image_index,
            frame_state_index,
            frame_state.command_buffer
        );
    }

    void FrameManager::present(Token& token) noexcept {
        auto& frame_state = frame_states[frame_state_index];

        const vk::PipelineStageFlags wait_dst_stage_mask[] = {
            vk::PipelineStageFlagBits::eColorAttachmentOutput
        };

        const auto submit_info = vk::SubmitInfo()
            .setCommandBuffers(*frame_state.command_buffer)
            .setWaitSemaphores(*frame_state.image_available_semaphore)
            .setSignalSemaphores(*frame_state.render_finished_semaphore)
            .setWaitDstStageMask(wait_dst_stage_mask);
        queue.submit(submit_info);

        const auto present_info = vk::PresentInfoKHR()
            .setWaitSemaphores(*frame_state.render_finished_semaphore)
            .setSwapchains(*swap_chain_context.swap_chain)
            .setImageIndices(token.image_index);

        frame_state_index = (frame_state_index + 1) % (swap_chain_context.images.size() + 1);
    }

    std::expected<FrameManager, Error> FrameManager::create(
        const vk::raii::PhysicalDevice& physical_device,
        vk::raii::Device& device, 
        vk::raii::Queue& queue,
        vulkan::context::SwapChainContext context
    ) noexcept {
        const auto frame_state_count = context.images.size() + 1;

        std::vector<FrameManager::FrameState> frame_states;
        frame_states.reserve(frame_state_count);

        const auto command_pool_info = vk::CommandPoolCreateInfo()
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

        return FrameManager(
            physical_device,
            device,
            queue,
            std::move(context),
            std::move(command_pool),
            std::move(frame_states)
        );
    }
}