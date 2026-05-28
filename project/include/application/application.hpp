#pragma once


#include <cstddef>
#include <memory>
#include <optional>
#include <vulkan/vulkan.hpp>

#include "event_bus.hpp"
#include "platform/window.hpp"
#include "utils/obj_model.hpp"
#include "vulkan/context/device.hpp"
#include "vulkan/object_mgmt./resource/memory/free_list_memory_pool.hpp"
#include "vulkan/object_mgmt/frame/frame_mgmt.hpp"
#include "vulkan/object_mgmt/descriptor.hpp"
#include "vulkan/object_mgmt/render_pass.hpp"
#include "vulkan/object_mgmt/pipeline.hpp"
#include "vulkan/object_mgmt/resource/buffer.hpp"
#include <vulkan/context/instace.hpp>

class Application {
    using MempryPoolType = vulkan::object_mgmt::FreeListMemoryPool<
    vulkan::object_mgmt::detail::AllocatorStrategy::BestFit>;
    private:
        struct Resources {
            std::optional<ObjModel> model;
            vulkan::object_mgmt::Buffer vertex_buffer;
            vulkan::object_mgmt::Buffer index_buffer;
            vulkan::object_mgmt::Buffer uniform_buffer;
            vulkan::object_mgmt::DescriptorPool descriptor_pool;
            vulkan::object_mgmt::DescriptorSet descriptor_set;
        };

        EventBus ebus;
        platform::WindowContext window_context;
        std::unique_ptr<vulkan::context::InstanceContext> instace_context;
        std::unique_ptr<vulkan::context::DeviceContext> device_context;

        MempryPoolType mem_pool;
        vulkan::object_mgmt::RenderPass pass;
        vulkan::object_mgmt::frame::FrameManager frame_manager;
        vulkan::object_mgmt::DescriptorSetLayout set_layout;
        vulkan::object_mgmt::PipeLine pipeline;

        std::unique_ptr<Resources> resource{nullptr};
        
    public:
        Application(
            EventBus event_bus,
            platform::WindowContext window_context,
            std::unique_ptr<vulkan::context::InstanceContext> instance_context,
            std::unique_ptr<vulkan::context::DeviceContext> device_context,
            MempryPoolType mem_pool,
            vulkan::object_mgmt::RenderPass pass,
            vulkan::object_mgmt::frame::FrameManager frame_manager,
            vulkan::object_mgmt::DescriptorSetLayout set_layout,
            vulkan::object_mgmt::PipeLine pipeline
        ) noexcept : 
            ebus(std::move(event_bus)),
            window_context(std::move(window_context)),
            instace_context(std::move(instance_context)),
            device_context(std::move(device_context)),
            mem_pool(std::move(mem_pool)),
            pass(std::move(pass)),
            frame_manager(std::move(frame_manager)),
            set_layout(std::move(set_layout)),
            pipeline(std::move(pipeline))
        {};

        ~Application() noexcept;

        void init();

        bool loop();

        static Application create();
};