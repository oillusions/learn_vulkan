#include "vulkan/object_mgmt/descriptor.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <map>
#include <ranges>

#include <utils/utils.hpp>

namespace vulkan::object_mgmt {

    DescriptorSetLayout::Builder& 
    DescriptorSetLayout::Builder::append(
        vk::DescriptorSetLayoutBinding bind_item
    ) noexcept {
        bind_item.setBinding(item_count++);
        bind_items.push_back(std::move(bind_item));

        return *this;
    }

    std::expected<DescriptorSetLayout, Error> 
    DescriptorSetLayout::Builder::build(
        vk::raii::Device& device
    ) noexcept {
        if (bind_items.empty()) return Error("构建描述符集布局失败: 集项为空");

        const auto bind_items_array = bind_items | std::ranges::to<std::vector>();

        const auto layout_info = vk::DescriptorSetLayoutCreateInfo()
            .setBindings(bind_items_array);

        auto result_layout = device.createDescriptorSetLayout(layout_info)
            | Error::from("创建描述符集布局对象失败");
        if (!result_layout) return result_layout.error();
    

        return DescriptorSetLayout(
            device,
            std::move(result_layout.value()),
            std::move(bind_items)
        );
    }

    std::vector<vk::DescriptorPoolSize> 
    DescriptorSetLayout::get_pool_sizes() const noexcept {
        std::map<vk::DescriptorType, vk::DeviceSize> type_count;
        for (const auto& item : bind_items) {
            type_count[item.descriptorType] += item.descriptorCount;
        }

        std::vector<vk::DescriptorPoolSize> pool_sizes;
        pool_sizes.reserve(type_count.size());
        for (const auto& [type, count] : type_count) {
            pool_sizes.push_back(vk::DescriptorPoolSize{type, static_cast<uint32_t>(count)});
        }

        return pool_sizes;
    }

    const std::vector<vk::DescriptorSetLayoutBinding>&
    DescriptorSetLayout::get_bindings() const noexcept {
        return bind_items;
    }

    std::expected< DescriptorPool, Error> 
    DescriptorSetLayout::create_pool(const vk::DeviceSize max_sets) noexcept {
        return DescriptorPool::builder()
            .maxSets(max_sets)
            .append(*this)
            .build(device);

    }

    DescriptorSetLayout::Builder DescriptorSetLayout::builder() noexcept {
        return DescriptorSetLayout::Builder();
    }

    DescriptorPool::Builder& DescriptorPool::Builder::append(
        const DescriptorSetLayout& layout, 
        const vk::DeviceSize count
    ) noexcept {
        if (count > 0) {
            statistical_info.push_back(statistical{layout, count});
        }
        return *this;
    }

    DescriptorPool::Builder& DescriptorPool::Builder::maxSets(vk::DeviceSize size) noexcept {
        if (size > 0) {
            this->max_sets = size;
        }
        return *this;
    }

    std::expected<DescriptorPool, Error> 
    DescriptorPool::Builder::build(vk::raii::Device& device) noexcept {
        if (statistical_info.empty()) return Error("布局统计信息为空");
        std::vector<vk::DescriptorPoolSize> sizes;
        for (const auto& info : statistical_info) {
            const auto& layout = info.layout;
            auto layout_sizes = layout.get_pool_sizes();
            for (auto& size : layout_sizes) {
                size.descriptorCount = size.descriptorCount * info.count * max_sets;
            }
            sizes.append_range(layout_sizes);
        }

        std::map<vk::DescriptorType, vk::DeviceSize> type_count;

        for (const auto& size : sizes) {
            type_count[size.type] += size.descriptorCount;
        }

        sizes.clear();
        for (const auto& [type, count] : type_count) {
            sizes.push_back(vk::DescriptorPoolSize{
                type, 
                static_cast<uint32_t>(count)
            });
        }

        const auto pool_info = vk::DescriptorPoolCreateInfo()
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setPoolSizes(sizes)
            .setMaxSets(max_sets);

        auto result_pool = device.createDescriptorPool(pool_info)
            | Error::from("创建描述符池对象失败");
        if (!result_pool) return result_pool.error();

        return DescriptorPool(
            device,
            std::move(result_pool.value())
        );
    }

    std::expected<DescriptorSet, Error> 
    DescriptorPool::allocate(DescriptorSetLayout& layout) noexcept {
        const auto allocate_info = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(pool)
            .setSetLayouts(**layout);

        auto result_descriptor_set = device.allocateDescriptorSets(allocate_info)
            | Error::from("分配描述符集失败");
        if (!result_descriptor_set) return result_descriptor_set.error();

        return DescriptorSet(
            device,
            layout,
            std::move(result_descriptor_set.value()[0])
        );
    }

    std::expected<std::vector<DescriptorSet>, Error> 
    DescriptorPool::allocateMultple() noexcept {
        return Error("代码未完工");
    }

    DescriptorPool::Builder DescriptorPool::builder() noexcept {
        return DescriptorPool::Builder();
    }

    void DescriptorSet::writeItem(vk::WriteDescriptorSet write_info) noexcept {
        write_info.setDstSet(set);
        device.updateDescriptorSets(write_info, nullptr);


    }
}