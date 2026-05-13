#pragma once


#include <vulkan/vulkan.hpp>

#include "event_bus.hpp"
#include "platform/window.hpp"
#include <vulkan/context/instace.hpp>

class Application {
    private:
        EventBus ebus;
        platform::WindowContext window_context;
        vulkan::context::InstanceContext instace_context;
        
    public:
        Application(
            EventBus event_bus,
            platform::WindowContext window_context,
            vulkan::context::InstanceContext instance_context
        ) noexcept : 
            ebus(std::move(event_bus)),
            window_context(std::move(window_context)),
            instace_context(std::move(instance_context))
        {};

        ~Application() noexcept;

        void init() noexcept;

        bool loop() noexcept;

        static Application create();
};