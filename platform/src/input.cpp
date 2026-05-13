#include "platform/input.hpp"
#include <GLFW/glfw3.h>

namespace platform::input {
    inline EventBus& get_event_bus(GLFWwindow* window) noexcept {
        return *static_cast<EventBus*>(glfwGetWindowUserPointer(window));
    }

    void init_platform_event(EventBus &event_bus, const WindowContext &context) noexcept {
        glfwSetWindowUserPointer(context.window, static_cast<void*>(&event_bus));

        glfwSetKeyCallback(context.window, [] (GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
            const auto content = event::types::KeyboardEventContent(window, key, scancode, action, mods);
            event::func::key_callback(get_event_bus(window), content);
        });
        glfwSetScrollCallback(context.window, [] (GLFWwindow* window, const double x_offset, const double y_offset) {
            const auto content = event::types::MouseScrollEventContent(window, x_offset, y_offset);
            event::func::mouse_scroll_callback(get_event_bus(window), content);
        });

        glfwSetCursorPosCallback(context.window, [] (GLFWwindow* window, const double x, const double y) {
            const auto content = event::types::CursorMoveEventContent(window, x, y);
            event::func::cursor_move_callback(get_event_bus(window), content);
        });

        glfwSetMouseButtonCallback(context.window, [] (GLFWwindow* window, const int button, const int action, const int mods) {
            const auto content = event::types::MouseButtonEventContent(window, button, action, mods);
            event::func::mouse_button_callback(get_event_bus(window), content);
        });
        glfwSetWindowSizeCallback(context.window, [] (GLFWwindow* window, const int width, const int height) {
            const auto content = event::types::FrameResizeEventContent(window, width, height);
            event::func::window_size_callback(get_event_bus(window), content);
        });
        glfwSetWindowCloseCallback(context.window, [] (GLFWwindow* window) {
            const auto content = event::types::WindowCloseEventContent{window};
            event::func::window_close_callback(get_event_bus(window), content);
        });
    }
}