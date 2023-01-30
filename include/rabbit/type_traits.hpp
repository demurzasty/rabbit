#pragma once 

#include <type_traits>

namespace rb {
    /**
     * @brief In C++20 is_pod and is_pod_v will be deprecated. 
     *        Provide our own implementation if is_pod and is_pod_v. 
     */
    template<class T>
    struct is_pod {
        static constexpr bool value = std::is_standard_layout_v<T> && std::is_trivial_v<T>;
    };

    template<class T>
    constexpr bool is_pod_v = is_pod<T>::value;
}
