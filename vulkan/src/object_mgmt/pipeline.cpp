#include "vulkan/object_mgmt/pipeline.hpp"
#include "vulkan/object_mgmt/descriptor.hpp"

#include <filesystem>
#include <fstream>

#include <utils/utils.hpp>
#include <vulkan/vulkan_core.h>
#include "vulkan/object_mgmt/shader.hpp"

namespace fs = std::filesystem;

const auto shader_path = fs::path("./resource/shader");

constexpr std::string vert_shader_name = "default.vert.spv";
constexpr std::string frag_shader_name = "default.frag.spv";

inline std::expected<std::vector<uint32_t>, Error> load_shader_code(const fs::path& path) noexcept {
    if (!fs::exists(path)) Error("路径不存在");
    
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return Error(
        "着色器文件打开失败", 
        std::format("路径:{}", path.string())
    );
    const size_t file_size = file.tellg();
    if (file_size == 0) return Error(
        "着色器文件为空",
        std::format("路径:{}", path.string())
    );
    if (file_size % 4 != 0) return Error(
        "着色器文件非四字节对齐",
        std::format("路径:{}", path.string())
    );

    std::vector<uint32_t> ret;
    ret.resize(file_size / 4);

    file.seekg(0);
    file.read(reinterpret_cast<std::istream::char_type *>(ret.data()), file_size);

    return ret;
}

namespace vulkan::object_mgmt {
    std::expected<PipeLine, Error> PipeLine::create(
        vk::raii::Device& device,
        RenderPass& pass,
        DescriptorSetLayout& set_layout
    ) noexcept {

        auto result_vert_shader_code = load_shader_code(shader_path / vert_shader_name);
        if (!result_vert_shader_code) return result_vert_shader_code
            .error().forward("加载顶点着色器代码失败");

        auto result_frag_shader_code = load_shader_code(shader_path / frag_shader_name);
        if (!result_frag_shader_code) return result_vert_shader_code
            .error().forward("加载顶点着色器代码失败");

        auto result_vert_shader = Shader::create(
            device,
            result_vert_shader_code.value()
        );
        if (!result_vert_shader) return result_vert_shader
            .error().forward("顶点着色器创建失败");

        auto result_frag_shader = Shader::create(
            device,
            result_frag_shader_code.value()
        );
        if (!result_frag_shader) return result_frag_shader
            .error().forward("片段着色器创建失败");

        const auto shader_stage_info = std::array {
            vk::PipelineShaderStageCreateInfo()
                .setStage(vk::ShaderStageFlagBits::eVertex)
                .setModule(*result_vert_shader.value())
                .setPName("main"),

            vk::PipelineShaderStageCreateInfo()
                .setStage(vk::ShaderStageFlagBits::eFragment)
                .setModule(*result_frag_shader.value())
                .setPName("main"),
        };

        const auto layout_info = vk::PipelineLayoutCreateInfo()
            .setSetLayouts(**set_layout);

        auto result_layout = device.createPipelineLayout(layout_info)
            | Error::from("管线布局创建失败");
        auto pipeline_layout = std::move(result_layout).value();


        const auto dynamic_states = std::array {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        const auto dynamic_state_info = vk::PipelineDynamicStateCreateInfo()
            .setDynamicStates(dynamic_states);


        const auto binding_rescription = vk::VertexInputBindingDescription()
            .setBinding(0)
            .setStride(sizeof(Vertex))
            .setInputRate(vk::VertexInputRate::eVertex);

        const auto vertex_attribute_rescriptions = std::array {
            vk::VertexInputAttributeDescription()
                .setBinding(0)
                .setLocation(0)
                .setFormat(vk::Format::eR32G32B32Sfloat)
                .setOffset(offsetof(Vertex, vertex)),
            vk::VertexInputAttributeDescription()
                .setBinding(0)
                .setLocation(1)
                .setFormat(vk::Format::eR32G32B32Sfloat)
                .setOffset(offsetof(Vertex, color)),
            vk::VertexInputAttributeDescription()
                .setBinding(0)
                .setLocation(2)
                .setFormat(vk::Format::eR32G32Sfloat)
                .setOffset(offsetof(Vertex, uv))
        };

        const auto binding_descriptions = std::array {
            binding_rescription
        };

        const auto attribute_rescriptions = vertex_attribute_rescriptions;

        const auto vertex_input_state_info = vk::PipelineVertexInputStateCreateInfo()
            .setVertexBindingDescriptions(binding_descriptions)
            .setVertexAttributeDescriptions(attribute_rescriptions);
        

        const auto vertex_assemebly_state_info = vk::PipelineInputAssemblyStateCreateInfo()
            .setTopology(vk::PrimitiveTopology::eTriangleList)
            .setPrimitiveRestartEnable(vk::False);

        
        const auto rasterization_state_info = vk::PipelineRasterizationStateCreateInfo()
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1.0f)
            .setCullMode(vk::CullModeFlagBits::eNone)
            .setFrontFace(vk::FrontFace::eClockwise)

            .setDepthClampEnable(vk::False)
            .setDepthBiasEnable(vk::False)
            .setDepthBiasClamp(0.0f)
            .setDepthBiasSlopeFactor(0.0f)
            .setDepthBiasConstantFactor(0.0f);
        
        
        const auto muilisample_state_info = vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setSampleShadingEnable(vk::False)
            .setMinSampleShading(0.0f)
            .setPSampleMask(nullptr)
            .setAlphaToCoverageEnable(vk::False)
            .setAlphaToOneEnable(vk::False);

        const auto color_blend_attachment_state_info = vk::PipelineColorBlendAttachmentState()
            .setColorWriteMask(
                vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA
            )
            .setBlendEnable(vk::False);

        const auto color_blend_attachment_state_infos = std::array {
            color_blend_attachment_state_info
        };

        const auto color_blending_state_info = vk::PipelineColorBlendStateCreateInfo()
            .setLogicOpEnable(vk::False)
            .setAttachments(color_blend_attachment_state_infos);

        const auto viewport = vk::Viewport()
            .setWidth(10)
            .setHeight(10)
            .setMinDepth(0.0f)
            .setMaxDepth(0.0f);
    
        const auto scissor = vk::Rect2D()
            .setExtent(vk::Extent2D{10, 10});

        const auto viewport_state_info = vk::PipelineViewportStateCreateInfo()
            .setViewports(viewport)
            .setScissors(scissor);

        const auto pipeline_info = vk::GraphicsPipelineCreateInfo()
            .setPDynamicState(&dynamic_state_info)
            .setStages(shader_stage_info)
            .setLayout(pipeline_layout)
            .setPVertexInputState(&vertex_input_state_info)
            .setPInputAssemblyState(&vertex_assemebly_state_info)
            .setPRasterizationState(&rasterization_state_info)
            .setPMultisampleState(&muilisample_state_info)
            .setPColorBlendState(&color_blending_state_info)
            .setPViewportState(&viewport_state_info)
            .setRenderPass(*pass)
            .setSubpass(0);
        
        auto result_pipeline = device.createGraphicsPipeline(
            VK_NULL_HANDLE, 
            pipeline_info
        )   | Error::from("管线对象创建失败");

        if (!result_pipeline) return result_pipeline.error();

        return PipeLine(
            std::move(result_pipeline.value()),
            std::move(pipeline_layout)
        );
    }
}