#pragma once
#include <set>

#ifdef _WIN32
#define GLFW_INCLUDE_VULKAN
#endif
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

#include <error.hpp>

namespace platform {
    struct WindowConfig {
        std::string title{"undefined"};
        uint32_t width{800};
        uint32_t height{600};
        bool enable_resizable{true};
    };

    void init_platform() noexcept;

    class WindowContext {
        public:
            GLFWwindow* window;

            WindowContext(GLFWwindow* window) noexcept :
                window(window)
            {};

            WindowContext(WindowContext&& other) noexcept :
                window(other.window) {
                    other.window = nullptr;
            }
            WindowContext(const WindowContext&) = delete;

            WindowContext& operator = (WindowContext&& other) noexcept {
                if (this != &other) {
                    this->window = other.window;
                    other.window = nullptr;
                }
                return *this;
            }
            WindowContext& operator = (const WindowContext&) = delete;

            std::expected<vk::raii::SurfaceKHR, Error> createSurface(const vk::raii::Instance& instance) noexcept;

            static std::expected<WindowContext, Error> create(const WindowConfig& config) noexcept;
    };

    std::set<const char*> obtain_platform_extensions() noexcept;
    std::set<const char*> obtain_platform_layers() noexcept;
    
}