#pragma once

#include <list>

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

#include <error.hpp>


namespace vulkan::object_mgmt {

    enum class AttachmentUsage {
        Undefined = 0,
        Color,
        Input,
        Resolve,
        Preserve,
        DepthStencil
    };

    namespace detail {
        struct AttachmentReferenceInfo {
            uint32_t attachment{vk::SubpassExternal};
            vk::ImageLayout layout{vk::ImageLayout::eUndefined};
            AttachmentUsage usage{AttachmentUsage::Undefined};
        };
    };

    class RenderPass {
        public:
            class Builder {
                public:
                    class Attachment {
                        public:
                            friend Builder;
                            vk::AttachmentDescription& description;
                        private:
                            Builder& builder;
                            // vk::ImageLayout type;
                            uint32_t index;

                        private:
                            Attachment(
                                Builder& builder, 
                                // const vk::ImageLayout type, 
                                const uint32_t index,
                                vk::AttachmentDescription& description
                            ) noexcept :
                                description(description),
                                builder(builder),
                                // type(type),
                                index(index)
                            {};

                        public:

                            Attachment(const Attachment&) = delete;
                            Attachment& operator = (const Attachment&) = delete;

                            Attachment(Attachment&&) noexcept = default;
                        
                    };

                    class Subpass {
                        public:
                            friend Builder;
                            vk::SubpassDescription& description;

                        private:
                            Builder& builder;
                            uint32_t index;

                            std::vector<detail::AttachmentReferenceInfo> attachment_refs;
    

                            Subpass(
                                Builder& builder, 
                                const uint32_t index,
                                vk::SubpassDescription& description
                            ) noexcept :
                                description(description),
                                builder(builder),
                                index(index)
                            {};

                        public:
                            Subpass& appendAttachment(
                                const Attachment& attachment,
                                const vk::ImageLayout purpose,
                                AttachmentUsage usage
                            ) noexcept;

                            Subpass(const Subpass&) = delete;
                            Subpass& operator = (const Subpass&) = delete;

                            Subpass(Subpass&&) = default;

                    };

                private:
                    std::list<vk::AttachmentDescription> attachment_descriptions;
                    std::list<vk::SubpassDescription> subpass_descriptions;
                    std::list<Attachment> attachment_list;
                    std::list<Subpass> subpass_list;
                    
                    uint32_t attachment_count;
                    uint32_t subpass_count;

                public:

                    Builder() = default;

                    Attachment& appendAttachment(
                        // const vk::ImageLayout type
                    ) noexcept;
                    Subpass& appendSubpass() noexcept;

                    std::expected<RenderPass, Error> build(vk::raii::Device& device) noexcept;

            };
            friend Builder;

            vk::raii::Device& device;
            vk::raii::RenderPass pass;

        private:

            RenderPass(
                vk::raii::Device& device,
                vk::raii::RenderPass pass
            ) noexcept : 
                device(device),
                pass(std::move(pass))
            {};

        public:
            vk::raii::RenderPass& operator * () noexcept {
                return pass;
            }

            static Builder builder() noexcept;

    };
}