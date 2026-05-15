#pragma once


#include <vulkan/vulkan.hpp>

#include "event_bus.hpp"
#include "platform/window.hpp"
#include "vulkan/context/device.hpp"
#include "vulkan/object_mgmt/frame/frame_mgmt.hpp"
#include <vulkan/context/instace.hpp>

class Application {
    private:
        EventBus ebus;
        platform::WindowContext window_context;
        vulkan::context::InstanceContext instace_context;
        vulkan::context::DeviceContext device_context;
        vulkan::object_mgmt::frame::FrameManager frame_manager;
        
    public:
        Application(
            EventBus event_bus,
            platform::WindowContext window_context,
            vulkan::context::InstanceContext instance_context,
            vulkan::context::DeviceContext device_context,
            vulkan::object_mgmt::frame::FrameManager frame_manager
        ) noexcept : 
            ebus(std::move(event_bus)),
            window_context(std::move(window_context)),
            instace_context(std::move(instance_context)),
            device_context(std::move(device_context)),
            frame_manager(std::move(frame_manager))
        {};

        ~Application() noexcept;

        void init() noexcept;

        bool loop();

        static Application create();
};