#include "application/impl/device.hpp"

#include "utils/global_logger.hpp"
#include "utils/utils.hpp"
#include "project_config.hpp"
#include "vulkan/context/device.hpp"
#include "vulkan/utils/queue_family.hpp"

#include "vulkan/vulkan.hpp"
#include <ranges>

namespace application::impl {
    namespace device {
        std::set<const char*> obtain_essential_extensions() noexcept {
            return std::set<const char*>{
                vk::KHRSwapchainExtensionName,
            };
        }
    }

    inline std::expected<vk::raii::PhysicalDevice, Error> 
    select_physical_device(const vulkan::context::InstanceContext& instance_context) noexcept {
        auto result_physical_devices = instance_context.instance.enumeratePhysicalDevices()
            | Error::from("枚举硬件设备失败");
        if (!result_physical_devices) return result_physical_devices.error();
        auto physical_devices = std::move(result_physical_devices).value();

        return physical_devices[0];
    }

    vk::PhysicalDeviceFeatures obtain_device_features(
        const vk::raii::PhysicalDevice& physical_device
    ) noexcept {
        return vk::PhysicalDeviceFeatures();
    }

    std::set<vk::DeviceQueueCreateInfo> obtain_essential_queues(
        const vulkan::context::PhysicalDeviceContext& physical_device_context
    ) noexcept {
        std::string content{"\nqueue_families: \n"};
        for(const auto& queue_family : physical_device_context.queue_families) {
            content.append(std::format(
                "    flags: {} \n    count: {} \n\n", 
                vk::to_string(queue_family.properties.queueFlags), 
                queue_family.properties.queueCount)
            );
        }
        glog.log<LogLevel::Debug>(content);
        
        return std::set<vk::DeviceQueueCreateInfo>{};
    }

    inline vulkan::context::PhysicalDeviceContext
    create_physical_device_context(vk::raii::PhysicalDevice physical_device) noexcept {
        const auto queue_family_properties = physical_device.getQueueFamilyProperties();

        auto queue_family_counters = std::vector<vulkan::utils::QueueFamily>();
        queue_family_counters.reserve(queue_family_properties.size());
        for (const auto& [index, properters] : queue_family_properties | std::views::enumerate) {
            queue_family_counters.emplace_back(properters, index);
        }

        return vulkan::context::PhysicalDeviceContext(
            std::move(physical_device),
            std::move(queue_family_counters)
        );
    }

    inline std::expected<vk::raii::Device, Error> 
    create_device(
        vk::raii::PhysicalDevice& physical_device,
        const std::set<const char*>& enabled_extensions,
        const vk::PhysicalDeviceFeatures& enabled_features,
        const std::set<vk::DeviceQueueCreateInfo>& queues
    ) noexcept {

        const auto extension_array = enabled_extensions
            | std::views::transform([] (auto&& extension_name) {return extension_name;})
            | std::ranges::to<std::vector>();

        const auto queue_array = queues
            | std::views::transform([] (auto&& queue_info) {return queue_info;})
            | std::ranges::to<std::vector>();


        const auto device_info = vk::DeviceCreateInfo()
            .setPEnabledExtensionNames(extension_array)
            .setPEnabledFeatures(&enabled_features)
            .setQueueCreateInfos(queue_array);
        return physical_device.createDevice(device_info)
            | Error::from("创建逻辑设备对象失败");
    }
    
    std::expected<vulkan::context::DeviceContext, Error> 
    create_device_context(vulkan::context::InstanceContext& instance_context) noexcept {

        auto result_physical_device = select_physical_device(instance_context);
        if (!result_physical_device) return result_physical_device.error()
            .forward("未找到合适的硬件设备");
        auto physical_device_context = create_physical_device_context(
            std::move(result_physical_device).value()
        );

        const auto device_features = obtain_device_features(physical_device_context.physical_device);

        // const auto essential_queues = obtain_essential_queues(physical_device_context);

        const auto queue_priorities = std::vector{
            1.0f
        };

        auto graphical_queue = vk::DeviceQueueCreateInfo()
            .setQueueFamilyIndex(0)
            .setQueueCount(1)
            .setQueuePriorities(queue_priorities);

        const auto essential_queues = std::set{
            graphical_queue
        };

        const auto project_ectensions = project::vulkan::device::obtan_project_extensions();

        const auto essential_extensions = device::obtain_essential_extensions();

        const auto extensions = project_ectensions | essential_extensions;
    
        auto result_device = create_device(
            physical_device_context.physical_device,
            extensions,
            device_features,
            essential_queues
        );
        if (!result_device) return result_device.error().forward("创建逻辑设备失败");
        auto device = std::move(result_device).value();

        const auto queues = std::vector {
            device.getQueue(0, 0)
        };

        return vulkan::context::DeviceContext(
            std::move(physical_device_context),
            std::move(device),
            std::move(queues)
        );

        return Error("代码未完成");
    }
}