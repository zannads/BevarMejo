#pragma once
#ifdef ENABLE_SAFETY_CHECKS
#include <iostream>

template <typename T>
class SafeMemberPtr
{
private:
    T* m_ptr;

public:
    // Constructors
    SafeMemberPtr() noexcept : m_ptr(nullptr) {}
    explicit SafeMemberPtr(T* ptr) noexcept : m_ptr(ptr) {}
    SafeMemberPtr(const SafeMemberPtr&) noexcept = default;
    SafeMemberPtr(SafeMemberPtr&&) noexcept = default;

    // Accessors
    T& operator*() const noexcept { return *m_ptr; }
    T* operator->() const noexcept { return m_ptr; }
    T* get() const noexcept { return m_ptr; }

    // Extra method
    bool expired() const noexcept { return m_ptr == nullptr; }

    // Assignment operator for raw pointers
    SafeMemberPtr& operator=(const SafeMemberPtr&) noexcept = default;
    SafeMemberPtr& operator=(SafeMemberPtr&&) noexcept = default;
    SafeMemberPtr& operator=(T* ptr) noexcept
    {
        m_ptr = ptr;
        return *this;
    }

    // Conversion to bool for validity check
    explicit operator bool() const noexcept { return m_ptr != nullptr; }

    // Comparison operators
    bool operator==(const SafeMemberPtr& other) const noexcept { return m_ptr == other.m_ptr; }
    bool operator!=(const SafeMemberPtr& other) const noexcept { return m_ptr != other.m_ptr; }
    bool operator==(std::nullptr_t) const noexcept { return m_ptr == nullptr; }
    bool operator!=(std::nullptr_t) const noexcept { return m_ptr != nullptr; }
    bool operator<(const SafeMemberPtr& other) const noexcept { return m_ptr < other.m_ptr; }
    bool operator<=(const SafeMemberPtr& other) const noexcept { return m_ptr <= other.m_ptr; }
    bool operator>(const SafeMemberPtr& other) const noexcept { return m_ptr > other.m_ptr; }
    bool operator>=(const SafeMemberPtr& other) const noexcept { return m_ptr >= other.m_ptr; }
};

/*
// It is basically like a weak_ptr (so it register and unregister itself in the constructor and destructor)
// We don't have the lock method, because we can't own it. However, the user should use check_validity
//  to check if the object is still valid before using it.
// From claude::
// Smart pointer class for safe tracking of member objects
template<typename T>
class SafeMemberPtr {
private:
    // Underlying raw pointer
    T* ptr;

    // Pointer to the tracking object (if applicable)
    const SafePointableMember* tracker;

    // Atomic flag to track validity
    std::atomic<bool> is_valid;

public:
    // Default constructor
    SafeMemberPtr() : ptr(nullptr), tracker(nullptr), is_valid(false) {}

    // Constructor from raw pointer
    explicit SafeMemberPtr(T* p) : ptr(p), is_valid(p != nullptr) {
        // If the pointed object is derived from SafePointableMember
        tracker = dynamic_cast<const SafePointableMember*>(p);
        
        // Register if tracking is possible
        if (tracker) {
            tracker->register_member_ptr(this);
        }
    }

    // Rest of the implementation remains the same as in the previous artifact
    // ... (copy/move constructors, assignment operators, etc.)
};
*/
#endif // ENABLE_SAFETY_CHECKS
