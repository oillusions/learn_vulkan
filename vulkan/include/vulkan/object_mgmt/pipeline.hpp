#pragma once

#include <glm/matrix.hpp>
#include <optional>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <error.hpp>

#include "descriptor.hpp"
#include "render_pass.hpp"
#include "vulkan/vulkan.hpp"

namespace vulkan::object_mgmt {
    class PipeLine {
        public:
            struct VertexInfo {
                vk::PipelineVertexInputStateCreateInfo vertex_input_info;
                vk::PipelineInputAssemblyStateCreateInfo assembly_info;
            };

            struct MVP {
                glm::mat4 model;
                glm::mat4 view; 
                glm::mat4 proj;
            };

            struct Vertex {
                glm::vec3 vertex;
                glm::vec3 color;
                glm::vec2 uv;
            };

        private:
            vk::raii::Pipeline pipeline;
            vk::raii::PipelineLayout layout;
        private:
            PipeLine(
                vk::raii::Pipeline pipeline,
                vk::raii::PipelineLayout layout
            ) noexcept :
                pipeline(std::move(pipeline)),
                layout(std::move(layout))
            {}

        public:

            vk::raii::PipelineLayout& get_layout() noexcept {
                return layout;
            }

            vk::raii::Pipeline& operator * () noexcept {
                return pipeline;
            }

            static std::expected<PipeLine, Error> create(
                vk::raii::Device& device,
                RenderPass& pass,
                DescriptorSetLayout& set_layout,
                std::optional<VertexInfo> vertex_info = std::nullopt
            ) noexcept;
    };
}