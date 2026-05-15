#include "application/impl/instance.hpp"

#include <optional>
#include <ranges>

#include "platform/window.hpp"
#include "vulkan/context/instace.hpp"

#include "project_config.hpp"

#include "utils/utils.hpp"
#include "utils/global_logger.hpp"
#include "vulkan/vulkan.hpp"


namespace application::impl {
    namespace instance {
        std::set<const char *> obtain_essential_extensions(const InstanceConfig& config) noexcept {
            auto extensions = std::set<const char *>();
            if (config.enable_validation) {
                extensions.insert(vk::EXTDebugUtilsExtensionName);
            }
            return extensions;
        }
    
        std::set<const char *> obtain_essential_layers(const InstanceConfig& config) noexcept {
            auto layers = std::set<const char *>();
            if (config.enable_validation) {
                layers.insert("VK_LAYER_KHRONOS_validation");
            }
            return layers;
        }
    }

    inline VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
        vk::DebugUtilsMessageTypeFlagsEXT message_type,
        const vk::DebugUtilsMessengerCallbackDataEXT* callback_data_ptr,
        void* user_data_ptr
    ) noexcept {
        switch (message_severity) {
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose : {
                glog.log<LogLevel::Debug>(std::string(callback_data_ptr->pMessage));
                break;
            }
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo : {
                glog.log<LogLevel::Debug>(std::string(callback_data_ptr->pMessage));
                break;
            }
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning : {
                glog.log<LogLevel::Warn>(std::string(callback_data_ptr->pMessage));
                break;
            }
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError : {
                glog.log<LogLevel::Error>(std::string(callback_data_ptr->pMessage));
                break;
            }
        }
        return vk::False;
    }

    inline std::expected<vk::raii::Instance, Error> create_instance(
        vk::raii::Context& context,
        const std::set<const char *>& layers,
        const std::set<const char *>& extensions,
        const vk::ApplicationInfo& application_info
    ) noexcept {
        const auto layers_array = layers | std::ranges::to<std::vector>();
        const auto extensions_array = extensions | std::ranges::to<std::vector>();

        const auto instance_info = vk::InstanceCreateInfo()
            .setPApplicationInfo(&application_info)
            .setPEnabledLayerNames(layers_array)
            .setPEnabledExtensionNames(extensions_array);

        return context.createInstance(instance_info)
            | Error::from("创建实例对象失败");
    }

    std::expected<std::optional<vk::raii::DebugUtilsMessengerEXT>, Error> 
    create_debug_utils_messenger(
        vk::raii::Instance& instance,
        const InstanceConfig& config
    ) noexcept {
        if (config.enable_validation) {
            const auto messenger_info = vk::DebugUtilsMessengerCreateInfoEXT()
                .setMessageType(
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                    | vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding
                    | vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                    | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                )
                .setMessageSeverity(
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                    | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                    | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                    | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                )
                .setPfnUserCallback(debug_callback);

            return instance.createDebugUtilsMessengerEXT(messenger_info)
                | Error::from("创建信使对象失败");
        }
        return std::nullopt;
    }

    std::expected<vulkan::context::InstanceContext, Error> 
    create_instance_context(const InstanceConfig& config) noexcept {
        auto context = vk::raii::Context();

        const auto application_info = vk::ApplicationInfo()
            .setPApplicationName(config.application_name.c_str())
            .setApplicationVersion(config.application_version)
            .setPEngineName(config.engine_name.c_str())
            .setEngineVersion(config.engine_version)
            .setApiVersion(config.api_version);

        const auto platform_layers = platform::obtain_platform_layers();
        const auto platform_extensions = platform::obtain_platform_extensions();

        const auto project_layers = project::vulkan::instance::obtan_project_layer();
        const auto project_extensions = project::vulkan::instance::obtan_project_extensions();

        const auto essential_layers = instance::obtain_essential_layers(config);
        const auto essential_extensions = instance::obtain_essential_extensions(config);

        const auto extensions = platform_extensions | project_extensions | essential_extensions;
        const auto layers = platform_layers | project_layers | essential_layers;

        auto result_instance = create_instance(
            context, 
            layers, 
            extensions, 
            application_info
        );

        if (!result_instance)
            return result_instance.error().forward("实例创建失败");
        auto instance = std::move(result_instance).value();

        auto result_debug_utils_messenger = create_debug_utils_messenger(instance, config);
        if (!result_debug_utils_messenger) return result_debug_utils_messenger.error().forward("创建信使失败");
        auto debug_utils_messenger = std::move(result_debug_utils_messenger).value();
        return vulkan::context::InstanceContext(
            std::move(context),
            std::move(instance), 
            std::move(debug_utils_messenger)
        );
    }
}