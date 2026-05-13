#pragma once

#include <set>
namespace project {
    namespace vulkan {
        namespace instance {
            inline std::set<const char*> obtan_project_extensions() noexcept {
                return std::set<const char*>{};
            }
        
            inline std::set<const char*> obtan_project_layer() noexcept {
                return std::set<const char*>{};
            }
        }
        namespace device {
            inline std::set<const char*> obtan_project_extensions() noexcept {
                return std::set<const char*>{};
            }
        
            inline std::set<const char*> obtan_project_layer() noexcept {
                return std::set<const char*>{};
            }
        }
    }
}