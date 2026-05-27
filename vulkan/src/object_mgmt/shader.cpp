#include "vulkan/object_mgmt/shader.hpp"
#include <utils/utils.hpp>

namespace vulkan::object_mgmt {
    std::expected<Shader, Error> Shader::create(
        vk::raii::Device& device,
        const vk::ArrayProxyNoTemporaries<const uint32_t>& code,
        const vk::DeviceSize code_size
    ) noexcept {
        const auto shader_info =  vk::ShaderModuleCreateInfo()
            .setCode(code)
            .setCodeSize(code_size == 0 ? code.size() * sizeof(uint32_t) : code_size);

        auto result_shader = device.createShaderModule(shader_info)
            | Error::from("着色器模块对象创建失败");
        if (!result_shader) return result_shader.error();




        return Shader(
            std::move(result_shader.value())
        );
    }
}