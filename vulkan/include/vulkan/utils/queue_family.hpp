#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <error.hpp>

namespace vulkan::utils {
    class QueueFamily {
        private: 
            vk::DeviceSize queue_count;
            vk::DeviceSize queue_occupancy_count;
        public:
            const vk::DeviceSize queu_family_index;
            const vk::QueueFamilyProperties properties;

            class Possessor {
                public:
                    friend QueueFamily; 
                    QueueFamily& queue_family;
                    const vk::DeviceSize queue_occupancy_count;

                private:
                    bool is_release{false};

                    Possessor(QueueFamily& queue_family, const vk::DeviceSize occupany_count) noexcept :
                        queue_family(queue_family),
                        queue_occupancy_count(occupany_count)
                    {};

                public:
                    Possessor(Possessor&& other) noexcept :
                        queue_family(other.queue_family),
                        queue_occupancy_count(other.queue_occupancy_count),
                        is_release(other.is_release) {
                            other.is_release = true;
                    }

                    Possessor(const Possessor&) = delete;
                    Possessor& operator = (const Possessor&) = delete;

                    void deallocate() noexcept;

                    ~Possessor() noexcept;
            };
        
            QueueFamily(const vk::QueueFamilyProperties& properties, const vk::DeviceSize index) noexcept :
                properties(properties),
                queu_family_index(index),
                queue_count(properties.queueCount),
                queue_occupancy_count(0)
            {};

            ~QueueFamily() = default;

            vk::DeviceSize queueRemainCount() const noexcept;

            std::expected<Possessor, Error> allocate(const vk::DeviceSize count) noexcept;
            void deallocate(Possessor& pssosser) noexcept; 
    };
}