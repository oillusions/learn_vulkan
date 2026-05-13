#include <algorithm>
#include <iterator>
#include <set>

#include <error.hpp>
#include <vulkan/vulkan.hpp>

template <typename T>
std::set<T> operator | (const std::set<T> &l, const std::set<T> &r) noexcept {
    std::set<T> o;
    std::set_union(
        l.begin(), l.end(), 
        r.begin(), r.end(),
        std::inserter(o, o.begin())
    );
    return o;
}

template <typename T>
std::expected<T, Error> operator | (
    vk::ResultValue<T> result,
    Error::FromFunctor functor
) noexcept {
    if (result.result != vk::Result::eSuccess) {
        return functor(vk::to_string(result.result));
    }
    return std::move(result).value;
}