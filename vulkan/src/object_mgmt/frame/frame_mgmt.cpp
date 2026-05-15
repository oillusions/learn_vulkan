#include "vulkan/object_mgmt/frame/frame_mgmt.hpp"

#include <error.hpp>
#include "utils/utils.hpp"



namespace vulkan::object_mgmt::frame {
    FrameManager::FrameManager(
        vk::raii::Device& device,
        vulkan::context::SwapChainContext context,
        vk::raii::CommandPool command_pool,
        std::vector<FrameState> frame_states
        ) noexcept :
            device(device),
            swap_chain_context(std::move(context)),
            command_pool(std::move(command_pool)),
            frame_states(std::move(frame_states))
    {}

    std::expected<FrameManager, Error> FrameManager::create(
        vk::raii::Device &device, 
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
        if (!result_command_pool) return result_command_pool.error();
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
            device,
            std::move(context),
            std::move(command_pool),
            std::move(frame_states)
        );
    }
}