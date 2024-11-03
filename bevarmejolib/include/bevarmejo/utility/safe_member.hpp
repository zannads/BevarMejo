#pragma once 

#ifdef ENABLE_SAFETY_CHECKS
class SafeMember { };   // Empty class to be used as a base class
/*
#include <memory>
#include <unordered_set>
#include <atomic>
#include <mutex>

// Constructors should be protected to prevent direct instantiation but allow inheritance
// register_ptr, unregister_ptr and invalidate should be private and the pointer class should be a friend
// When a pointer is created, it is responsible for registering itself with the object
// When a pointer is destroyed, it is responsible for unregistering itself with the object
// When the object is destroyed, it is responsible for invalidating all pointers
// count method should be public and return the number of registered pointers

// Forward declaration
template<typename T>
class SafeMemberPtr;

// Base class for objects that can have safe member pointers
class SafeMember {
private:
    // Mutex to protect concurrent modifications
    mutable std::mutex member_mutex;

    // Set of all member pointers tracking this object
    mutable std::unordered_set<void*> member_ptrs;

public:
    // Register a member pointer
    void register_member_ptr(void* ptr) const {
        std::lock_guard<std::mutex> lock(member_mutex);
        member_ptrs.insert(ptr);
    }

    // Unregister a member pointer
    void unregister_member_ptr(void* ptr) const {
        std::lock_guard<std::mutex> lock(member_mutex);
        member_ptrs.erase(ptr);
    }

    // Invalidate all member pointers
    void invalidate_members() {
        std::lock_guard<std::mutex> lock(member_mutex);
        for (void* ptr : member_ptrs) {
            // Assuming SafeMemberPtr has a statisc method to invalidate
            reinterpret_cast<SafeMemberPtr<SafeMember>*>(ptr)->invalidate();
        }
        member_ptrs.clear();
    }

    // Virtual destructor to ensure proper cleanup
    virtual ~SafeMember() {
        invalidate_members();
    }
};
*/
#endif // ENABLE_SAFETY_CHECKS
