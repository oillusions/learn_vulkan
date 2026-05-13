#pragma once
#include <cstdint>
#include <set>

#include "vulkan/context/instace.hpp"

#include <error.hpp>


namespace application::impl {
    struct InstanceConfig {
        std::string application_name{"undefined"};
        std::string engine_name{"undefined"};
        uint32_t application_version{vk::makeVersion(1, 0, 0)};
        uint32_t engine_version{vk::makeVersion(1, 0, 0)};
        uint32_t api_version{vk::ApiVersion10};

        #ifdef NDEBUG
            static constexpr bool default_enable_validation = false;
        #else
            static constexpr bool default_enable_validation = true;
        #endif
        
        bool enable_validation = default_enable_validation;
    };

    namespace instance {
        std::set<const char*> 
        obtain_essential_extensions(const InstanceConfig& config) noexcept;
        
        std::set<const char*> 
        obtain_essential_layers(const InstanceConfig& config) noexcept;
    }

    std::expected<vulkan::context::InstanceContext, Error> 
    create_instance_context(const InstanceConfig& config) noexcept;
}