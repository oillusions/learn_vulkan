#include "vulkan/object_mgmt/render_pass.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <ranges>
#include <utils/utils.hpp>

namespace vulkan::object_mgmt {

    RenderPass::Builder::Attachment& 
    RenderPass::Builder::appendAttachment(
        // const vk::ImageLayout type
    ) noexcept {
        attachment_descriptions.emplace_back(vk::AttachmentDescription());
        attachment_list.emplace_back(
            RenderPass::Builder::Attachment(
                *this,
                // type,
                attachment_count++,
                attachment_descriptions.back()
            )
        );
        return attachment_list.back();
    }

    RenderPass::Builder::Subpass& 
    RenderPass::Builder::appendSubpass() noexcept {
        subpass_descriptions.emplace_back(vk::SubpassDescription());
        subpass_list.emplace_back(
            RenderPass::Builder::Subpass(
                *this, 
                subpass_count++,
                subpass_descriptions.back()
            )
        );

        return subpass_list.back();
    }

    RenderPass::Builder::Subpass& 
    RenderPass::Builder::Subpass::appendAttachment(
        const RenderPass::Builder::Attachment& attachment,
        const vk::ImageLayout purpose,
        AttachmentUsage usage
    ) noexcept {
        attachment_refs.emplace_back(attachment.index, purpose, usage);
        return *this;
    }

    struct SubpassAttachment {
        std::vector<vk::AttachmentReference> colors{};
        std::vector<vk::AttachmentReference> inputs{};
        std::vector<vk::AttachmentReference> resolves{};
        std::vector<uint32_t> preserves{};
        vk::AttachmentReference depth_stencil{UINT32_MAX, vk::ImageLayout::eUndefined};
    };

    std::expected<RenderPass, Error> 
    RenderPass::Builder::build(vk::raii::Device& device) noexcept {
        std::vector<SubpassAttachment> attachment_refs;
        std::vector<vk::SubpassDependency> subpass_dependencies;

        std::vector<detail::AttachmentReferenceInfo> prev_infos;
        std::vector<vk::DeviceSize> attachment_ref_counters;

        attachment_refs.resize(subpass_descriptions.size());
        prev_infos.resize(attachment_descriptions.size());
        attachment_ref_counters.resize(attachment_descriptions.size());

        for (const auto& subpass : subpass_list) {
            vk::DeviceSize attachment_count{0};
            for (auto& attachment : subpass.attachment_refs) {
                attachment_ref_counters[attachment.attachment] += 1;

                switch (attachment.usage) {
                    case AttachmentUsage::Color : {
                        attachment_refs[subpass.index].colors.emplace_back(
                            attachment.attachment,
                            attachment.layout
                        );
                        break;
                    }
                    case AttachmentUsage::Input : {
                        attachment_refs[subpass.index].inputs.emplace_back(
                            attachment.attachment,
                            attachment.layout
                        );
                        break;
                    }
                    case AttachmentUsage::Resolve : {
                        attachment_refs[subpass.index].resolves.emplace_back(
                            attachment.attachment,
                            attachment.layout
                        );
                        break;
                    }
                    case AttachmentUsage::Preserve : {
                        attachment_refs[subpass.index].preserves.emplace_back(
                            attachment.attachment
                        );
                        break;
                    }
                    case AttachmentUsage::DepthStencil : {
                        attachment_refs[subpass.index].depth_stencil = vk::AttachmentReference()
                            .setAttachment(attachment.attachment)
                            .setLayout(attachment.layout);
                        break;
                    }
                    default :;
                }

                attachment_count++;
            }

            

            for (auto& attachment : subpass.attachment_refs) {
                switch (attachment.usage) {
                    case AttachmentUsage::Color : {
                        if (prev_infos[attachment.attachment].attachment == vk::SubpassExternal) {
                            subpass_dependencies.emplace_back(
                                vk::SubpassDependency()
                                    .setSrcSubpass(vk::SubpassExternal)
                                    .setDstSubpass(subpass.index)
                                    .setSrcAccessMask(vk::AccessFlagBits::eNone)
                                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                                    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                                    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                            );
                        }
                        break;
                    }
                    default :;
                }
            }


            if (!attachment_refs[subpass.index].colors.empty()) {
                subpass.description.setColorAttachments(
                    attachment_refs[subpass.index].colors
                );
            }

            if (!attachment_refs[subpass.index].inputs.empty()) {
                subpass.description.setInputAttachments(
                    attachment_refs[subpass.index].inputs
                );
            }

            if (!attachment_refs[subpass.index].resolves.empty()) {
                subpass.description.setResolveAttachments(
                    attachment_refs[subpass.index].resolves
                );
            }

            if (!attachment_refs[subpass.index].preserves.empty()) {
                subpass.description.setPreserveAttachments(
                    attachment_refs[subpass.index].preserves
                );
            }
        

            if (attachment_refs[subpass.index].depth_stencil.attachment == UINT32_MAX) {
                subpass.description.setPDepthStencilAttachment(nullptr);
            } else {
                subpass.description.setPDepthStencilAttachment(
                    &attachment_refs[subpass.index].depth_stencil
                );
            }

            for (auto& attachment : subpass.attachment_refs) {
                prev_infos[attachment.attachment].attachment = subpass.index;
                prev_infos[attachment.attachment].layout = attachment.layout;
                prev_infos[attachment.attachment].usage = attachment.usage;
            }
        }

        const auto subpass_description_array = subpass_descriptions | std::ranges::to<std::vector>();
        const auto attachment_description_array = attachment_descriptions | std::ranges::to<std::vector>();

        const auto render_pass_info = vk::RenderPassCreateInfo()
            .setSubpasses(subpass_description_array)
            .setAttachments(attachment_description_array)
            .setDependencies(subpass_dependencies);

        auto result_render_pass = device.createRenderPass(render_pass_info)
            | Error::from("渲染通道对象创建失败");
        if (!result_render_pass) return result_render_pass.error();

        return RenderPass(
            device,
            std::move(result_render_pass.value())
        );
    }

    RenderPass::Builder RenderPass::builder() noexcept {
        return RenderPass::Builder();
    }
}