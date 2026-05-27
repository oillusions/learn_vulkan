#pragma once

#include "vulkan/vulkan.hpp"
#include <expected>
#include <list>

#include <vulkan/vulkan_raii.hpp>

#include <error.hpp>

namespace vulkan::object_mgmt {
    class DescriptorPool;


    class DescriptorSetLayout {
        public:
            class Builder {
                public:
                    Builder() = default;

                    Builder& append(vk::DescriptorSetLayoutBinding bind_item) noexcept;

                    std::expected<DescriptorSetLayout, Error> build(vk::raii::Device& device) noexcept;

                    private:
                    vk::DeviceSize item_count{0};
                    std::list<vk::DescriptorSetLayoutBinding> bind_items;

            };
            friend Builder;

            std::vector<vk::DescriptorPoolSize> get_pool_sizes() const noexcept;
            const std::list<vk::DescriptorSetLayoutBinding>& get_bindings() const noexcept; 
            std::expected<DescriptorPool, Error> create_pool(const vk::DeviceSize max_sets = 1) noexcept;

            vk::raii::DescriptorSetLayout& operator * () noexcept {
                return layout;
            }

            static Builder builder() noexcept;

        private:
            std::list<vk::DescriptorSetLayoutBinding> bind_items;
            vk::raii::Device& device;
            vk::raii::DescriptorSetLayout layout;

            DescriptorSetLayout(
                vk::raii::Device& device,
                vk::raii::DescriptorSetLayout layout,
                std::list<vk::DescriptorSetLayoutBinding> bind_items
            ) noexcept :
                device(device),
                layout(std::move(layout)),
                bind_items(std::move(bind_items))
            {};
    };

    class DescriptorSet;

    class DescriptorPool {
        public:
            class Builder {
                public:
                    Builder() = default;

                    Builder& append(const DescriptorSetLayout& layout, const vk::DeviceSize count = 1) noexcept;
                    Builder& maxSets(vk::DeviceSize size = 1) noexcept;

                    std::expected<DescriptorPool, Error> build(vk::raii::Device& device) noexcept;

                private:
                    struct statistical {
                        const DescriptorSetLayout& layout;
                        vk::DeviceSize count;
                    };

                    std::vector<statistical> statistical_info;
                    vk::DeviceSize max_sets{1};
            };
            friend Builder;

        private:
            vk::raii::Device& device;
            vk::raii::DescriptorPool pool;

        public:

            std::expected<DescriptorSet, Error> allocate(DescriptorSetLayout& layout) noexcept;
            std::expected<std::vector<DescriptorSet>, Error> allocateMultple() noexcept;

            vk::raii::DescriptorPool& operator * () noexcept {
                return pool;
            }

            static Builder builder() noexcept;

        private:

            DescriptorPool(
                vk::raii::Device& device,
                 vk::raii::DescriptorPool pool
            ) noexcept :
                device(device),
                pool(std::move(pool))
            {};
    };

    class DescriptorSet {
        private:
            vk::raii::Device& device;
            DescriptorSetLayout& layout;
            vk::raii::DescriptorSet set;

        public:

            void writeItem(vk::WriteDescriptorSet write_info) noexcept;

            vk::raii::DescriptorSet& operator * () noexcept {
                return set;
            }

        private:
            
            DescriptorSet(
                vk::raii::Device& device,
                DescriptorSetLayout& layout,
                vk::raii::DescriptorSet set
            ) noexcept : 
                device(device), 
                layout(layout),
                set(std::move(set))
            {}
            friend DescriptorPool;
    };
}