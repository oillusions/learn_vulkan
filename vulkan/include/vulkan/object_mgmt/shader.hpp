#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <error.hpp>

namespace vulkan::object_mgmt {
    class Shader {
        public:
            
        private:
            vk::raii::ShaderModule shader;

        public:
            Shader(vk::raii::ShaderModule shader) noexcept :
                shader(std::move(shader))    
            {};


            vk::raii::ShaderModule& operator * () noexcept {
                return shader;
            }

            static std::expected<Shader, Error> create(
                vk::raii::Device& device,
                const vk::ArrayProxyNoTemporaries<const uint32_t>& code,
                const vk::DeviceSize code_size = 0
            ) noexcept; 
    };
}