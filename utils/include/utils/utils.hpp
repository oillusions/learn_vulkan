#pragma once
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <concepts>
#include <expected>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

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


template<typename L, typename Func>
requires std::invocable<Func, L>
constexpr auto operator | (L&& l, Func&& func)
    noexcept(noexcept(std::forward<Func>(func)(std::forward<L>(l))))
    -> decltype(std::forward<Func>(func)(std::forward<L>(l))) {
    return std::forward<Func>(func)(std::forward<L>(l));
}

template<typename L, typename Func>
requires std::invocable<Func, const std::string&>
constexpr std::expected<L, Error> operator | (vk::ResultValue<L>&& result, Func&& func)
    noexcept(noexcept(std::forward<Func>(func)(std::move(vk::to_string(result.result))))) {
    if (result.result != vk::Result::eSuccess) {
        return std::forward<Func>(func)(std::move(vk::to_string(result.result)));
    } else {
        return std::expected<L, Error>(std::move(result.value));
    }
    
}


template<template<typename...> class ToType>
class WrapperTransfromFunctor {
    public:
        template<typename T>
        ToType<T> operator () (T&& value) const noexcept;

};

template<typename TargetType>
class TypeTransfromFunctor {
    public:
        template<typename T>
        TargetType operator () (T&& value) const noexcept;

};

template<>
class WrapperTransfromFunctor<std::unique_ptr> {
    public:
        template<typename T>
        requires std::same_as<T, std::remove_cvref_t<T>>
        constexpr auto operator () (T&& value) const noexcept
        -> decltype(std::make_unique<T>(std::forward<T>(value))) {
            return std::make_unique<T>(std::forward<T>(value));
        }

};


namespace utils {
    constexpr auto unique_ptr = WrapperTransfromFunctor<std::unique_ptr>{};
}
