#pragma once
#include "event_bus.hpp"
#include "platform/window.hpp"
#include <array>


namespace platform::input {
    class EventNames {
        public:
            enum Enum {
                eNull = 0,
                eFrameResize,
                eKeyboard,
                eMouseButton,
                eMouseScroll,
                eCursorMove,
                eWindowClose
            };

            static constexpr auto enum_values = std::array{
                "",
                "frame-resize-callback",
                "keyboard_callback",
                "mouse_button_callback",
                "mouse_scroll_callback",
                "cursor_move_callback",
                "window-close-callback"
            };

            Enum value{eNull};

            EventNames(const Enum enum_value) noexcept : 
                value(enum_value)
            {};

            constexpr std::string to_value() noexcept {
                return enum_values[this->value]; 
            }

            static constexpr std::string to_value(Enum enum_value) noexcept {
                return enum_values[enum_value];
            }
    };

    void init_platform_event(
        EventBus &event_bus,
        const WindowContext &window_context
    ) noexcept;

    namespace event {
        namespace types {
            struct FrameResizeEventContent {
                GLFWwindow *window{};
                int width{};
                int height{};
            };

            struct KeyboardEventContent {
                GLFWwindow *window{};
                int key{};
                int scancode{};
                int action{};
                int mods{};
            };

            struct MouseButtonEventContent {
                GLFWwindow *window{};
                int button;
                int action;
                int mods;
            };

            struct MouseScrollEventContent {
                GLFWwindow *window{};
                double x_offset;
                double y_offset;
            };

            struct CursorMoveEventContent {
                GLFWwindow *window{};
                double x{};
                double y{};
            };

            struct WindowCloseEventContent {
                GLFWwindow *window{};
            };
        }
        namespace func {
            inline void window_size_callback(
                const EventBus &event_bus,
                const types::FrameResizeEventContent &content
            ) noexcept {
                event_bus.publish(EventNames(EventNames::eFrameResize).to_value(), content);
            }

            inline void key_callback(
                const EventBus &event_bus,
                const types::KeyboardEventContent &content
            ) noexcept {
                event_bus.publish(EventNames(EventNames::eKeyboard).to_value(), content);
            }

            inline void mouse_button_callback(
                const EventBus &event_bus,
                const types::MouseButtonEventContent &content
            ) noexcept {
                event_bus.publish(EventNames(EventNames::eMouseButton).to_value(), content);
            }

            inline void mouse_scroll_callback(
                const EventBus &event_bus,
                const types::MouseScrollEventContent &content
            ) noexcept {
                event_bus.publish(EventNames(EventNames::eMouseScroll).to_value(), content);
            }

            inline void cursor_move_callback(
                const EventBus &event_bus,
                const types::CursorMoveEventContent &content
            ) noexcept {
                event_bus.publish(EventNames(EventNames::eCursorMove).to_value(), content);
            }

            inline void window_close_callback(
                const EventBus &event_bus,
                const types::WindowCloseEventContent &content
            ) noexcept {
                event_bus.publish(EventNames(EventNames::eWindowClose).to_value(), content);
            }
        }
    }
}
