#pragma once

#include <optional>
#include <vulkan/vulkan_raii.hpp>

namespace vulkan::context {
    class InstanceContext {
        public:
            vk::raii::Context context;
            vk::raii::Instance instance;
            std::optional<vk::raii::DebugUtilsMessengerEXT> messenger;

            InstanceContext(
                vk::raii::Context context,
                vk::raii::Instance instance,
                std::optional<vk::raii::DebugUtilsMessengerEXT> messenger    
            ) noexcept :
                context(std::move(context)),
                instance(std::move(instance)),
                messenger(std::move(messenger))
            {};  
    };
}