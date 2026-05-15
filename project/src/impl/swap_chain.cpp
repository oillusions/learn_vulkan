#include "application/impl/swap_chain.hpp"

#include <algorithm>
#include <vulkan/vulkan_format_traits.hpp>

#include "utils/utils.hpp"
#include "vulkan/context/swap_chain.hpp"

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

    inline vk::ComponentMapping
    obtain_format_componentMapping(const vk::Format format) {
        switch (format) {
            case vk::Format::eB8G8R8A8Unorm :  
            case vk::Format::eB8G8R8A8Snorm :  
            case vk::Format::eB8G8R8A8Uscaled :
            case vk::Format::eB8G8R8A8Sscaled :
            case vk::Format::eB8G8R8A8Uint :   
            case vk::Format::eB8G8R8A8Sint :   
            case vk::Format::eB8G8R8A8Srgb : {
                return vk::ComponentMapping()
                    .setR(vk::ComponentSwizzle::eB)
                    .setG(vk::ComponentSwizzle::eG)
                    .setB(vk::ComponentSwizzle::eR)
                    .setA(vk::ComponentSwizzle::eA);
            }  

            case vk::Format::eR8G8B8A8Unorm :  
            case vk::Format::eR8G8B8A8Snorm :  
            case vk::Format::eR8G8B8A8Uscaled :
            case vk::Format::eR8G8B8A8Sscaled :
            case vk::Format::eR8G8B8A8Uint :   
            case vk::Format::eR8G8B8A8Sint :   
            case vk::Format::eR8G8B8A8Srgb : {
                return vk::ComponentMapping()
                    .setR(vk::ComponentSwizzle::eR)
                    .setG(vk::ComponentSwizzle::eG)
                    .setB(vk::ComponentSwizzle::eB)
                    .setA(vk::ComponentSwizzle::eA);
            }
            default : {
                return vk::ComponentMapping()
                    .setR(vk::ComponentSwizzle::eR)
                    .setG(vk::ComponentSwizzle::eG)
                    .setB(vk::ComponentSwizzle::eB)
                    .setA(vk::ComponentSwizzle::eA);
            }
        }
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
        const vk::SurfaceCapabilitiesKHR& surface_capabilities
    ) noexcept {
        auto surface_supprted_composite_alpha_flags = surface_capabilities
            .supportedCompositeAlpha;

        if (surface_supprted_composite_alpha_flags & vk::CompositeAlphaFlagBitsKHR::eOpaque) {
            return vk::CompositeAlphaFlagBitsKHR::eOpaque;
        } else if (surface_supprted_composite_alpha_flags & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) {
            return vk::CompositeAlphaFlagBitsKHR::ePostMultiplied;
        } else if (surface_supprted_composite_alpha_flags & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) {
            return vk::CompositeAlphaFlagBitsKHR::ePreMultiplied;
        } else if (surface_supprted_composite_alpha_flags & vk::CompositeAlphaFlagBitsKHR::eInherit) {
            return vk::CompositeAlphaFlagBitsKHR::eInherit;
        }
        return Error("表面不支持任何支持的表面透明度模式");
    }

    inline vk::Extent2D obtain_image_extent(
        const vk::SurfaceCapabilitiesKHR& surface_capabilities
    ) noexcept {
        const auto& min_image_extent = surface_capabilities.minImageExtent;
        const auto& max_image_extent = surface_capabilities.maxImageExtent;
        const auto& curr_image_extent = surface_capabilities.currentExtent;

        const auto available_width = std::clamp(
            curr_image_extent.width, min_image_extent.width, max_image_extent.width
        );
        const auto available_height = std::clamp(
            curr_image_extent.height, min_image_extent.height, max_image_extent.height
        );

        return vk::Extent2D(available_width, available_height);
    }

    inline uint32_t obtain_present_required_image_count(
        const vk::PresentModeKHR& present_mode
    ) noexcept {
        switch (present_mode) {
            case vk::PresentModeKHR::eFifo : {
                return 2;
            }
            case vk::PresentModeKHR::eMailbox : {
                return 3;
            }
            case vk::PresentModeKHR::eFifoRelaxed : {
                return 2;
            }
            case vk::PresentModeKHR::eImmediate : {
                return 1;
            }
            default : {
                return 2;
            }
        }
    }

    inline vk::DeviceSize obtain_image_count(
        const vk::SurfaceCapabilitiesKHR& surface_capabilities,
        const vk::PresentModeKHR& surface_present_mode
    ) noexcept {
        const auto& min_image_count = surface_capabilities.minImageCount;
        const auto& max_image_count = surface_capabilities.maxImageCount;

        return std::clamp(
            obtain_present_required_image_count(surface_present_mode),
            min_image_count, max_image_count
        );
    }

    inline std::expected<vk::raii::SwapchainKHR, Error> 
    create_swap_chain(
        vk::raii::Device& device,
        const vk::raii::PhysicalDevice& physical_device,
        const vk::raii::SurfaceKHR& surface,
        const vk::SurfaceFormatKHR& surface_format,
        const vk::SurfacePresentModeKHR& surface_present_mode,
        const vk::CompositeAlphaFlagBitsKHR& surface_composite_alpha_mode,
        const vk::DeviceSize& image_count,
        const vk::Extent2D& image_extent
    ) noexcept {
        const auto swap_chain_info = vk::SwapchainCreateInfoKHR()
            .setSurface(surface)
            .setPresentMode(surface_present_mode.presentMode)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageFormat(surface_format.format)
            .setImageColorSpace(surface_format.colorSpace)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setImageArrayLayers(1)
            .setMinImageCount(image_count)
            .setImageExtent(image_extent)
            .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
            .setClipped(vk::True)
            .setCompositeAlpha(surface_composite_alpha_mode);

        return device.createSwapchainKHR(swap_chain_info)
            | Error::from("创建交换链对象失败");
    }

    std::expected<vulkan::context::SwapChainConfig, Error>
    obtain_swap_chain_config(
        const vk::raii::PhysicalDevice& physical_device,
        const vk::raii::SurfaceKHR& surface,
        const SwapChainConfig& config
    ) noexcept {
        auto result_surface_format = select_surface_format(
            physical_device, surface);
        if (!result_surface_format) return result_surface_format.error().forward("挑选可用的表面格式失败");
        const auto surface_format = result_surface_format.value();

        auto result_surface_present_mode = select_surface_present_mode(
            physical_device, surface);
        if (!result_surface_present_mode) return result_surface_present_mode.error().forward("挑选可用的表面呈现模式失败");
        const auto surface_present_mode = std::move(result_surface_present_mode).value();

        auto result_surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(surface)
            | Error::from("获取表面支持的能力信息失败");
        if (!result_surface_capabilities) return result_surface_capabilities.error();
        const auto surface_capabilities = std::move(result_surface_capabilities).value();

        const auto image_count = obtain_image_count(
            surface_capabilities, 
            surface_present_mode.presentMode
        );

        const auto image_extent = obtain_image_extent(
            surface_capabilities
        );

        auto result_surface_composite_alpha_flag_bits = select_surface_composite_alpha_mode(
            surface_capabilities
        );
        if (!result_surface_composite_alpha_flag_bits) return result_surface_present_mode.error().forward("挑选可用的表面透明度模式失败");
        const auto surface_composite_alpha_flag_bits = result_surface_composite_alpha_flag_bits.value();

        return vulkan::context::SwapChainConfig{
            .surface_format = surface_format,
            .present_mode = surface_present_mode.presentMode,
            .image_count = image_count,
            .image_extent = image_extent,
            .composite_alpha_flag_bits = surface_composite_alpha_flag_bits
        };
    }

    std::expected<vulkan::context::SwapChainContext, Error> 
    create_swap_chain_context(
        vulkan::context::DeviceContext &device_context, 
        vk::raii::SurfaceKHR surface,
        const SwapChainConfig& config
    ) noexcept {

        auto result_swap_chain_info = obtain_swap_chain_config(
            device_context.physical_device.physical_device, 
            surface, config
        );
        if (!result_swap_chain_info) return result_swap_chain_info.error().forward("获取交换链配置失败");
        const auto swap_chain_info = std::move(result_swap_chain_info).value();
    
        
        auto result_swap_chain = create_swap_chain(
            device_context.device,
            device_context.physical_device.physical_device,
            surface,
            swap_chain_info.surface_format,
            swap_chain_info.present_mode,
            swap_chain_info.composite_alpha_flag_bits,
            swap_chain_info.image_count,
            swap_chain_info.image_extent
        );
        if (!result_swap_chain) return result_swap_chain.error().forward("创建交换链失败");
        auto swap_chain = std::move(result_swap_chain).value();

        auto result_images = swap_chain.getImages()
            | Error::from("获取交换链图像组失败");
        if (!result_images) return result_images.error();
        auto images = std::move(result_images).value();

        auto image_view_info = vk::ImageViewCreateInfo()
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(swap_chain_info.surface_format.format)
            .setComponents(
                obtain_format_componentMapping(swap_chain_info.surface_format.format)
            )
            .setSubresourceRange(
                vk::ImageSubresourceRange()
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setBaseArrayLayer(0)
                    .setLevelCount(1)
                    .setLayerCount(1)
            );

        std::vector<vk::raii::ImageView> image_views;
        image_views.reserve(images.size());
        for (auto& image : images) {
            image_view_info.setImage(image);

            auto result_image_view = device_context.device.createImageView(image_view_info)
                | Error::from("创建交换链图像的图像视图失败");
            if (!result_image_view) return result_image_view.error();

            image_views.emplace_back(std::move(result_image_view).value());
        }

        
        return vulkan::context::SwapChainContext(
            std::move(surface),
            std::move(swap_chain),
            std::move(images),
            std::move(image_views),
            swap_chain_info
        );
    }
}