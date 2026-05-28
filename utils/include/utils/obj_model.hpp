#pragma once

#include "vulkan/vulkan.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include <error.hpp>


class ObjModel {
    public:
        struct Vertex {
            glm::vec3 vertex;
            glm::vec2 tex_coord;
        };

        struct ModelResource {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
        };


        ObjModel(
            std::string name,
            ModelResource resource
        ) noexcept;

        static vk::VertexInputBindingDescription obtain_vertex_bind_description() noexcept;
        static std::vector<vk::VertexInputAttributeDescription> obtain_vertex_attribute_description() noexcept;
        static vk::PipelineInputAssemblyStateCreateInfo obtain_input_assembly_info() noexcept;

        static std::expected<std::vector<ObjModel>, Error> create() noexcept;

    public:
        std::string name;
        ModelResource resource;
        


};
