#pragma once 

#include <memory>

namespace bevarmejo
{

template <typename T>
bool valid(T* ptr)
{
    return ptr != nullptr;
}

template <typename T>
bool valid(const std::unique_ptr<T>& ptr)
{
    return ptr != nullptr;
}

template <typename T>
bool valid(const std::shared_ptr<T>& ptr)
{
    return ptr != nullptr;
}

template <typename T>
bool valid(const std::weak_ptr<T>& ptr)
{
    return !ptr.expired();
}

// Thanks to https://www.fluentcpp.com/2021/05/14/a-default-value-to-dereference-null-pointers/
template <typename T, typename U>
decltype(auto) value_or(T* pointer, U&& default_value)
{
    return pointer ? *pointer : std::forward<U>(default_value);
}

template <typename T, typename U>
decltype(auto) value_or(const std::unique_ptr<T>& pointer, U&& default_value)
{
    return pointer ? *pointer : std::forward<U>(default_value);
}

template <typename T, typename U>
decltype(auto) value_or(const std::shared_ptr<T>& pointer, U&& default_value)
{
    return pointer ? *pointer : std::forward<U>(default_value);
}

template <typename T, typename U>
decltype(auto) value_or(const std::weak_ptr<T>& pointer, U&& default_value)
{
    return pointer.expired() ? std::forward<U>(default_value) : *pointer.lock();
}

template <typename T>
decltype(auto) value_or_empty(T* pointer)
{
    return pointer ? *pointer : T{};
}

template <typename T>
decltype(auto) value_or_empty(const std::unique_ptr<T>& pointer)
{
    return pointer ? *pointer : T{};
}

template <typename T>
decltype(auto) value_or_empty(const std::shared_ptr<T>& pointer)
{
    return pointer ? *pointer : T{};
}

template <typename T>
decltype(auto) value_or_empty(const std::weak_ptr<T>& pointer)
{
    return pointer.expired() ? T{} : *pointer.lock();
}

} // namespace bevarmejo
