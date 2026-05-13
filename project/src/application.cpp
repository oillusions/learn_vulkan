#include <application/application.hpp>

#include <GLFW/glfw3.h>

#include "common/global_logger.hpp"

#include "platform/input.hpp"
#include "platform/window.hpp"

#include "application/impl/instance.hpp"
#include "application/impl/device.hpp"
#include "application/impl/swap_chain.hpp"



Application Application::create() {
    platform::init_platform();

    EventBus event_bus;

    auto window_context = platform::WindowContext::create({}) 
        | Error::unwrap("窗口上下文创建失败");

    auto instance_context = application::impl::create_instance_context({})
        | Error::unwrap("vulkan实例上下文创建失败");

    auto surface = window_context.createSurface(instance_context.instance)
        | Error::unwrap("表面创建失败");

    auto device_context = application::impl::create_device_context(instance_context)
        | Error::unwrap("逻辑设备上下文创建失败");

    auto swap_chain_context = application::impl::create_swap_chain_context(device_context, std::move(surface))
        | Error::unwrap("交换链上下文创建失败");


     return Application(
        std::move(event_bus), 
        std::move(window_context),
        std::move(instance_context)
    );
}

Application::~Application() noexcept {
    glog.log<LogLevel::Info>("应用已退出");
}

void Application::init() noexcept {
    platform::input::init_platform_event(ebus, window_context);

    ebus.subscribe<platform::input::event::types::CursorMoveEventContent>(
        platform::input::EventNames(platform::input::EventNames::eCursorMove).to_value(),
        [](const platform::input::event::types::CursorMoveEventContent &content) {
            int width, height, pos_x, pos_y;
            glfwGetWindowSize(content.window, &width, &height);
            glfwGetWindowPos(content.window, &pos_x, &pos_y);
            glfwSetWindowPos(content.window, pos_x + (static_cast<int>(content.x) - (width / 2)), pos_y + (static_cast<int>(content.y) - (height / 2)));
    });

    ebus.subscribe<platform::input::event::types::KeyboardEventContent>(
        platform::input::EventNames(platform::input::EventNames::eKeyboard).to_value(),
        [] (const platform::input::event::types::KeyboardEventContent& content) {
            if (content.key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(content.window, GLFW_TRUE);
            }
    });

    glog.log<LogLevel::Info>("应用已启动");
}

bool Application::loop() noexcept {
    glfwPollEvents();
    return !glfwWindowShouldClose(window_context.window);
};