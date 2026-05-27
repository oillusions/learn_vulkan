#include "vulkan/object_mgmt/resource/buffer.hpp"
#include <utils/utils.hpp>

namespace vulkan::object_mgmt {
    std::expected<Buffer, Error> Buffer::create(
        vk::raii::Device& device,
        MemoryPool &pool, const 
        vk::BufferCreateInfo &info
    ) noexcept {
        auto result_buffer = device.createBuffer(info)
            | Error::from("缓冲区对象创建失败");
        if (!result_buffer) return result_buffer.error();
        auto buffer = std::move(result_buffer).value();

        auto result_allocation = pool.allocate(
            buffer.getMemoryRequirements()
        );
        if (!result_allocation) return result_allocation.error().forward("内存分配失败");
        auto allocation = std::move(result_allocation).value();

        buffer.bindMemory(pool.getDeviceMemory(), allocation.info.offset);

        return Buffer(
            std::move(allocation),
            std::move(buffer)
        );
    }
}