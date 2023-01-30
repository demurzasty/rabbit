#pragma once 

#include <memory>

namespace rb {
    /**
     * Creates a T object initialized with arguments p_args... at given address p_ptr.
     * 
     * @note Waiting for C++20.
     */
    template<typename T, class... Args>
    constexpr T* construct_at(T* p_ptr, Args&&... p_args) {
        return ::new (const_cast<void*>(static_cast<const volatile void*>(p_ptr))) 
            T(std::forward<Args>(p_args)...);
    }
}
