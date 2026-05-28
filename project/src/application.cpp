#include <application/application.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <optional>

#include "application/impl/device.hpp"
#include "application/impl/instance.hpp"
#include "application/impl/swap_chain.hpp"
#include "application/impl/mem.hpp"
#include "error.hpp"
#include "utils/global_logger.hpp"

#include "platform/input.hpp"
#include "platform/window.hpp"
#include "utils/obj_model.hpp"
#include "vulkan/object_mgmt/pipeline.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace vulkan::object_mgmt;

constexpr auto vertices = std::array {
    PipeLine::Vertex{
        glm::vec3(0.0f, -0.5f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec2(0.5f, 1.0f),
    },
    PipeLine::Vertex{
        glm::vec3(0.5f, 0.5f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
    },
    PipeLine::Vertex{
        glm::vec3(-0.5f, 0.5f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
    }
};

constexpr auto indices = std::array {
    0, 1, 2
};

Application Application::create() {
    platform::init_platform();

    EventBus event_bus;
    
    auto window_context = platform::WindowContext::create({}) 
        | Error::unwrap("窗口上下文创建失败");

    auto instance_context = application::impl::create_instance_context({})
        | Error::unwrap("vulkan实例上下文创建失败")
        | utils::unique_ptr;

    auto surface = window_context.createSurface(**instance_context)
        | Error::unwrap("表面创建失败");

    auto device_context = application::impl::create_device_context(*instance_context)
        | Error::unwrap("逻辑设备上下文创建失败")
        | utils::unique_ptr;

    auto swap_chain_context = application::impl::create_swap_chain_context(*device_context, std::move(surface), {})
        | Error::unwrap("交换链上下文创建失败");

    auto mem_pool = application::impl::create_memory_pool<MempryPoolType>(
        *device_context,
        vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
        2 << 16
    )   | Error::unwrap("内存池创建失败");

    auto set_layout = DescriptorSetLayout::builder()
        .append(
            vk::DescriptorSetLayoutBinding()
            .setBinding(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eVertex)
        ).build(**device_context)
            | Error::unwrap("描述符集布局创建失败");

    
    auto render_pass_builder = RenderPass::builder();
    auto& attachment = render_pass_builder.appendAttachment();

    attachment.description
        .setFormat(swap_chain_context.config.surface_format.format)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    auto& subpass_0 = render_pass_builder.appendSubpass();

    subpass_0.appendAttachment(
        attachment, 
        vk::ImageLayout::eColorAttachmentOptimal,
        AttachmentUsage::Color
    );

    auto render_pass = render_pass_builder.build(**device_context)
        | Error::unwrap("渲染通道创建失败");

    auto frame_manager = frame::FrameManager::create(
        device_context->physical_device.physical_device,
        **device_context,
        device_context->queues[0],
        std::move(swap_chain_context),
        render_pass
    )   | Error::unwrap("帧管理器创建失败");

    // const auto vertex_attribute_info = ObjModel::obtain_vertex_attribute_description();
    // const auto vertex_binding_info = ObjModel::obtain_vertex_bind_description();
    // const auto input_assembly_info = ObjModel::obtain_input_assembly_info();

    // const auto vertex_info = PipeLine::VertexInfo {
    //     vk::PipelineVertexInputStateCreateInfo()
    //         .setVertexAttributeDescriptions(vertex_attribute_info)
    //         .setVertexBindingDescriptions(vertex_binding_info),
    //     input_assembly_info
    // };

    auto pipeline = PipeLine::create(
        **device_context, 
        render_pass,
        set_layout
    )   | Error::unwrap("渲染管线创建失败");

    return Application(
        std::move(event_bus), 
        std::move(window_context),
        std::move(instance_context),
        std::move(device_context),
        std::move(mem_pool),
        std::move(render_pass),
        std::move(frame_manager),
        std::move(set_layout),
        std::move(pipeline)
    );
}

Application::~Application() noexcept {
    device_context->device.waitIdle();
    glog.log<LogLevel::Info>("应用已退出");
}

void Application::init() {
    platform::input::init_platform_event(ebus, window_context);

    ebus.subscribe<platform::input::event::types::KeyboardEventContent>(
        platform::input::EventNames(platform::input::EventNames::eKeyboard).to_value(),
        [] (const platform::input::event::types::KeyboardEventContent& content) {
            if (content.key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(content.window, GLFW_TRUE);
            }
    });

    ebus.subscribe<platform::input::event::types::FrameResizeEventContent>(
        platform::input::EventNames(platform::input::EventNames::eFrameResize).to_value(),
        [this] (const platform::input::event::types::FrameResizeEventContent& content) {
            frame_manager.rebuild(pass)
                | Error::unwrap("重建帧管理器失败");
    });

    auto obj_models = ObjModel::create()
        | Error::unwrap("加载obj模型失败");
    auto obj_model = std::move(obj_models)[0];

    auto vertex_buffer = Buffer::create(
        **device_context,
        mem_pool,
        vk::BufferCreateInfo()
            .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setSize(sizeof(vertices))
    )   | Error::unwrap("顶点缓冲区创建失败");
    vertex_buffer.load_buffer_host(vertices)
        | Error::unwrap("顶点加载失败");

    auto index_buffer = Buffer::create(
        **device_context,
        mem_pool,
        vk::BufferCreateInfo()
            .setUsage(vk::BufferUsageFlagBits::eIndexBuffer)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setSize(sizeof(indices))
    )   | Error::unwrap("索引缓冲区创建失败");
    index_buffer.load_buffer_host(indices)
        | Error::unwrap("索引加载失败");

    auto uniform_buffer= Buffer::create(
        **device_context,
        mem_pool,
        vk::BufferCreateInfo()
            .setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setSize(sizeof(PipeLine::MVP))
    )   |Error::unwrap("uniform缓冲区创建失败");

    auto descriptor_pool = set_layout.create_pool()
        | Error::unwrap("描述符池创建失败");
    auto set = descriptor_pool.allocate(set_layout)
        | Error::unwrap("描述符集创建失败");

    const auto buffer_info = vk::DescriptorBufferInfo()
        .setBuffer(*uniform_buffer)
        .setOffset(0)
        .setRange(sizeof(PipeLine::MVP));

    set.writeItem(
        vk::WriteDescriptorSet()
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setBufferInfo(buffer_info)
        .setDstSet(*set)
        .setDstBinding(0)
        .setDstArrayElement(0)
    );

    resource = std::make_unique<Resources>(Resources{
        std::nullopt,
        std::move(vertex_buffer),
        std::move(index_buffer),
        std::move(uniform_buffer),
        std::move(descriptor_pool),
        std::move(set)
    });

    glog.log<LogLevel::Info>("应用已启动");
}

bool Application::loop() {
    glfwPollEvents();
    
    if (!glfwWindowShouldClose(window_context.window)) {
        if (resource == nullptr) return true;

        auto result_token = frame_manager.obtain_frame_command_buffer();
        if (!result_token) {
            frame_manager.rebuild(pass)
                | Error::unwrap("重建帧管理器失败");
            return true;
        }
        auto token = std::move(result_token).value()
            | Error::unwrap("获取指令缓冲区失败");

        const auto command_buffer_begin_info = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        const auto clear_color = vk::ClearColorValue()
            .setFloat32({0.0f, 0.0f, 0.0f, 1.0f});

        const auto clear_info = vk::ClearValue()
            .setColor(clear_color);

        const auto frame_size = frame_manager.obtain_frame_size();

        const auto render_pass_begin_info = vk::RenderPassBeginInfo()
            .setRenderPass(*pass)
            .setFramebuffer(token.frame_buffer)
            .setClearValues(clear_info)
            .setRenderArea(vk::Rect2D()
                .setExtent(frame_size)
            );
        
        auto& cmd = token.command_buffer;
        auto& rs = *resource.get();

        uint32_t offset = 0;

        const auto viewprot = vk::Viewport()
            .setWidth(frame_size.width)
            .setHeight(frame_size.height)
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);
        const auto scissor = vk::Rect2D()
            .setExtent(frame_size);

        cmd.reset();
        cmd.begin(command_buffer_begin_info);
        cmd.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
        cmd.bindVertexBuffers(0, **rs.vertex_buffer, offset);
        cmd.bindIndexBuffer(*rs.index_buffer, 0, vk::IndexType::eUint32);
        cmd.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, 
            pipeline.get_layout(),
            0,
            **rs.descriptor_set,
            nullptr
        );
        cmd.setViewport(0, viewprot);
        cmd.setScissor(0, scissor);

        cmd.drawIndexed(indices.size(), 1, 0, 0, 0);

        cmd.endRenderPass();
        cmd.end();

        token.present();

    } else {
        return false;
    }
    return true;
};