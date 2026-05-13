#include "platform/window.hpp"

namespace platform {
    void init_platform() noexcept {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    std::expected<WindowContext, Error> WindowContext::create(const WindowConfig &config) noexcept {
        if (config.enable_resizable) {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        } else {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        }

        auto window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
        if (!window) {
            const char* description;
            glfwGetError(&description);
             return Error("窗口创建失败", std::string(description));
        }
        return WindowContext(window);
    }

    std::expected<vk::raii::SurfaceKHR, Error> WindowContext::createSurface(const vk::raii::Instance& instace) noexcept {
        VkSurfaceKHR surface;
        auto result = glfwCreateWindowSurface(*instace, this->window, nullptr, &surface);
        if (result != VK_SUCCESS) return Error("由glfw提供的创建表面函数创建失败", vk::to_string(static_cast<vk::Result>(result)));
        return vk::raii::SurfaceKHR(instace, surface);
    }

    std::set<const char*> obtain_platform_extensions() noexcept {
        uint32_t count;
        auto content = glfwGetRequiredInstanceExtensions(&count);

        if (count == 0) return std::set<const char*>{};

        return std::set(content, content + count);
    }

    std::set<const char*> obtain_platform_layers() noexcept {
        return std::set<const char*>{};
    }
}