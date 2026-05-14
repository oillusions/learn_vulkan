#include "application/impl/swap_chain.hpp"

#include <algorithm>
#include <vulkan/vulkan_format_traits.hpp>

#include "common/utils.hpp"
#include "vulkan/context/swap_chain.hpp"
#include "vulkan/vulkan.hpp"

namespace application::impl {

    inline vk::DeviceSize score_surface_format(
        const vk::SurfaceFormatKHR& surface_format
    ) noexcept {
        vk::DeviceSize score_count{0};
        if (surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            score_count += 50;
        }

        switch (surface_format.format) {
            case vk::Format::eB8G8R8A8Srgb : {
                score_count += 20;
                break;
            }
            case vk::Format::eR8G8B8A8Srgb : {
                score_count += 15;
                break;
            }
            case vk::Format::eB8G8R8A8Unorm : {
                score_count += 10;
                break;
            }
            case vk::Format::eR8G8B8A8Unorm : {
                score_count += 5;
                break;
            }
            default:;
        }
        return score_count;
    }

    inline std::expected<vk::SurfaceFormatKHR, Error> 
    select_surface_format(
        const vk::raii::PhysicalDevice& physical_device, 
        const vk::raii::SurfaceKHR& surface
    ) noexcept {
        const auto result_surface_formats = physical_device.getSurfaceFormatsKHR(surface)
            | Error::from("获取表面支持格式列表失败");
        if (!result_surface_formats) return result_surface_formats.error();
        auto surface_formats = std::move(result_surface_formats).value();
    
        std::vector<vk::SurfaceFormatKHR> after_basic_filter;
        for (const auto& surface_format : surface_formats) {
            if (surface_format.format == vk::Format::eUndefined) continue;
            if (
                vk::hasDepthComponent(surface_format.format)
                || vk::hasStencilComponent(surface_format.format)
                || vk::isCompressed(surface_format.format)) continue;

            after_basic_filter.push_back(surface_format);
        }

        if (after_basic_filter.empty()) return Error("无通过基础过滤可用的表面格式");
        
        return *std::max_element(
            after_basic_filter.begin(), after_basic_filter.end(), 
            [] (const auto& a, const auto& b) {
            return score_surface_format(a) < score_surface_format(b);
        });
    }

    inline vk::DeviceSize score_surface_present_mode(
        const vk::SurfacePresentModeKHR& surface_present_mode
    ) noexcept {
        auto score_count{0};
        switch (surface_present_mode.presentMode) {
            case vk::PresentModeKHR::eFifo : {
                score_count += 20;
                break;
            }
            case vk::PresentModeKHR::eMailbox : {
                score_count += 15;
                break;
            }
            case vk::PresentModeKHR::eFifoRelaxed : {
                score_count += 10;
                break;
            }
            case vk::PresentModeKHR::eImmediate : {
                score_count += 5;
            }
            default:;
        }
        return score_count;
    }

    inline std::expected<vk::SurfacePresentModeKHR, Error>
    select_surface_present_mode(
        const vk::raii::PhysicalDevice& physical_device, 
        const vk::raii::SurfaceKHR& surface
    ) noexcept {
        const auto result_surface_present_modes = physical_device.getSurfacePresentModesKHR(surface)
            | Error::from("获取表面支持呈现模式列表失败");
        if (!result_surface_present_modes) return result_surface_present_modes.error();
        auto surface_present_modes = std::move(result_surface_present_modes).value();

        return *std::max_element(
            surface_present_modes.begin(), surface_present_modes.end(), 
            [] (const auto& a, const auto& b
            ) {
            return score_surface_present_mode(a) < score_surface_present_mode(b);
        });
    }

    inline std::expected<vk::CompositeAlphaFlagBitsKHR, Error>
    select_surface_composite_alpha_mode(
        const vk::raii::PhysicalDevice& physical_device,
        const vk::raii::SurfaceKHR& surface
    ) noexcept {
        auto result_surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(surface)
            | Error::from("获取表面支持的能力信息失败");
        if (!result_surface_capabilities) return result_surface_capabilities.error();
        auto surface_supprted_composite_alpha_modes = std::move(result_surface_capabilities).value().supportedCompositeAlpha;

        if (surface_supprted_composite_alpha_modes & vk::CompositeAlphaFlagBitsKHR::eOpaque) {
            return vk::CompositeAlphaFlagBitsKHR::eOpaque;
        } else if (surface_supprted_composite_alpha_modes & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) {
            return vk::CompositeAlphaFlagBitsKHR::ePostMultiplied;
        } else if (surface_supprted_composite_alpha_modes & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) {
            return vk::CompositeAlphaFlagBitsKHR::ePreMultiplied;
        } else if (surface_supprted_composite_alpha_modes & vk::CompositeAlphaFlagBitsKHR::eInherit) {
            return vk::CompositeAlphaFlagBitsKHR::eInherit;
        }
        return Error("表面不支持任何支持的表面透明度模式");
    }



    inline std::expected<vk::raii::SwapchainKHR, Error> 
    create_swap_chain(
        vk::raii::Device& device,
        const vk::raii::PhysicalDevice& physical_device,
        const vk::raii::SurfaceKHR& surface,
        const vk::SurfaceFormatKHR& surface_format,
        const vk::SurfacePresentModeKHR& surface_present_mode,
        const vk::CompositeAlphaFlagBitsKHR& surface_composite_alpha_flags
    ) noexcept {
        const auto swap_chain_info = vk::SwapchainCreateInfoKHR()
            .setSurface(surface)
            .setPresentMode(surface_present_mode.presentMode)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageFormat(surface_format.format)
            .setImageColorSpace(surface_format.colorSpace)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setImageArrayLayers(1)
            .setMinImageCount(4)
            .setImageExtent({})
            .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
            .setClipped(vk::True)
            .setCompositeAlpha(surface_composite_alpha_flags);

        return device.createSwapchainKHR(swap_chain_info)
            | Error::from("创建交换链对象失败");
    }

    std::expected<vulkan::context::SwapChainContext, Error> 
    create_swap_chain_context(
        vulkan::context::DeviceContext &device_context, 
        vk::raii::SurfaceKHR surface,
        const SwapChainConfig& config
    ) noexcept {

        auto result_surface_format = select_surface_format(
            device_context.physical_device.physical_device, surface);
        if (!result_surface_format) return result_surface_format.error().forward("挑选可用的表面格式失败");
        const auto surface_format = result_surface_format.value();

        auto result_surface_present_mode = select_surface_present_mode(
            device_context.physical_device.physical_device, surface);
            if (!result_surface_present_mode) return result_surface_present_mode.error().forward("挑选可用的表面呈现模式失败");
        const auto surface_present_mode = std::move(result_surface_present_mode).value();

        auto result_surface_composite_alpha_flags = select_surface_composite_alpha_mode(
            device_context.physical_device.physical_device, surface);
        const auto surface_composite_alpha_flags = result_surface_composite_alpha_flags.value();

        auto result_swap_chain = create_swap_chain(
            device_context.device,
            device_context.physical_device.physical_device,
            surface,
            surface_format,
            surface_present_mode,
            surface_composite_alpha_flags
        );
        if (!result_swap_chain) return result_swap_chain.error().forward("创建交换链失败");
        auto swap_chain = std::move(result_swap_chain).value();

        return Error("代码未完成");
        
        // return vulkan::context::SwapChainContext{
        //     std::move(surface),
        //     std::move(swap_chain),

        // }
    }
}