#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_VULKAN

#include "error.hpp"

#include "utils/global_logger.hpp"
#include "application/application.hpp"


int main(int argc, char** argv) {
    global_logger::minLevel = LogLevel::Debug;

    try {
        auto app = Application::create();
        app.init();
        
        while(app.loop());

    } catch(const Error& error) {
        glog.log<LogLevel::Error>(error.string());
        return -1;
    }

    return 0;
}