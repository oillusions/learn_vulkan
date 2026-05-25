#include <application/application.hpp>

#include <GLFW/glfw3.h>

#include "utils/global_logger.hpp"

#include "platform/input.hpp"
#include "platform/window.hpp"

#include "vulkan/object_mgmt/frame/frame_mgmt.hpp"
#include "vulkan/object_mgmt/render_pass.hpp"

#include "application/impl/instance.hpp"
#include "application/impl/device.hpp"
#include "application/impl/swap_chain.hpp"
#include "vulkan/vulkan.hpp"


using namespace vulkan::object_mgmt;

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

    auto swap_chain_context = application::impl::create_swap_chain_context(device_context, std::move(surface), {})
        | Error::unwrap("交换链上下文创建失败");

    auto frame_manager = frame::FrameManager::create(
        device_context.physical_device.physical_device,
        device_context.device,
        device_context.queues[0],
        std::move(swap_chain_context)
    )   | Error::unwrap("帧管理器创建失败");

    auto render_pass_builder = RenderPass::builder();
    auto attachment = render_pass_builder.appendAttachment();

    attachment.description
        .setFormat(swap_chain_context.config.surface_format.format)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    auto subpass_0 = render_pass_builder.appendSubpass();

    subpass_0.appendAttachment(
        attachment, 
        vk::ImageLayout::eColorAttachmentOptimal,
        AttachmentUsage::Color
    );

    auto render_pass = render_pass_builder.build(device_context.device)
        | Error::unwrap("渲染通道创建失败");

    return Application(
        std::move(event_bus), 
        std::move(window_context),
        std::move(instance_context),
        std::move(device_context),
        std::move(frame_manager)
    );
}

Application::~Application() noexcept {
    glog.log<LogLevel::Info>("应用已退出");
}

void Application::init() noexcept {
    platform::input::init_platform_event(ebus, window_context);

    ebus.subscribe<platform::input::event::types::KeyboardEventContent>(
        platform::input::EventNames(platform::input::EventNames::eKeyboard).to_value(),
        [] (const platform::input::event::types::KeyboardEventContent& content) {
            if (content.key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(content.window, GLFW_TRUE);
            }
    });

    glog.log<LogLevel::Info>("应用已启动");
}

bool Application::loop() {
    glfwPollEvents();
    return !glfwWindowShouldClose(window_context.window);
};