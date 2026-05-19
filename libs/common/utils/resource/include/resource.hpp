#pragma once
#include <forward_list>
#include <memory>
#include <string>

#include <error.hpp>
#include <unordered_map>

/**
 * @brief 资源类接口
 */
class Resource {
public:
    Resource() = default;
    Resource(const Resource&) = delete;
    Resource& operator = (const Resource&) = delete;
    Resource(Resource&&) = default;
    Resource& operator = (Resource&&) = default;
    ~Resource() = default;
};

template<typename ResourceType>
class ResourceWrapper : public Resource {
    private:
        ResourceType value;
    public:
        using Type = ResourceType;

        template<typename... Args>
        explicit ResourceWrapper(Args&&... args) :
            value(std::forward<Args>(args)...)
        {};

        explicit ResourceWrapper(ResourceType&& value) noexcept :
            value(std::move(value))
        {};

        ~ResourceWrapper() = default;

        operator ResourceType& () {
            return value;
        }

        decltype(auto) get(this auto& self) noexcept {
            return std::forward_list<decltype(self)>(self.value);
        }

        auto operator -> (this auto& self) noexcept {
            return &self.value;
        }
};

template<typename ResourceType>
requires std::derived_from<ResourceType, Resource>
class TypedResourceManager {
    private:
        std::unordered_map<std::string, std::unique_ptr<ResourceType>> resource_map;
    public:
        using Type = ResourceType;

        TypedResourceManager() noexcept = default;
        ~TypedResourceManager() noexcept = default;

        TypedResourceManager(TypedResourceManager&&) noexcept = default;
        TypedResourceManager(TypedResourceManager&) = delete;
        TypedResourceManager& operator = (TypedResourceManager&&) noexcept = default;
        TypedResourceManager& operator = (TypedResourceManager&) = delete;

        template<typename... Args>
        std::expected<std::reference_wrapper<ResourceType>, Error> load(const std::string& key, Args&&... args) noexcept {
            if (resource_map.contains(key)) return find(key);
            try {
                auto [it, inserted] = resource_map.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(key),
                    std::forward_as_tuple(std::make_unique<ResourceType>(std::forward<Args>(args)...))
                );
                if (inserted) {
                    return std::ref(*it->second);
                }
                return Error("资源已存在, 加载失败");

            } catch (Error& e) {
                return e.forward("资源加载失败");
            }
        }

        template<typename... Args>
        std::expected<std::reference_wrapper<ResourceType>, Error> reload(const std::string& key, Args&&... args) noexcept {
            if (resource_map.contains(key)) {
                release(key);
                return load(key, std::forward<Args>(args)...);
            }
            return load(key, std::forward<Args>(args)...);
        }

        std::expected<std::reference_wrapper<ResourceType>, Error> find(const std::string& key) & noexcept {
            auto it = resource_map.find(key);
            if (it != resource_map.end()) {
                return std::ref(*it->second);
            }
            return Error("资源不存在");
        }

        std::expected<ResourceType, Error> find(const std::string& key) && {
            auto it = resource_map.find(key);
            if (it != resource_map.end()) {
                ResourceType value = std::move(*it->second);
                resource_map.erase(key);
                return value;
            }
            return Error("资源不存在");
        }

        std::expected<std::reference_wrapper<const ResourceType>, Error> find(const std::string& key) const & noexcept {
            auto it = resource_map.find(key);
            if (it != resource_map.end()) {
                return std::cref(*it->second);
            }
            return Error("资源不存在");
        }

        void release(const std::string& key) noexcept {
            if (resource_map.contains(key)) {
                resource_map.erase(key);
            }
        }
};