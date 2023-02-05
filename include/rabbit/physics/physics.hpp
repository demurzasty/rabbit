#pragma once 

#include <memory>

namespace rb {
    /** 
     * @brief Physics service.
     */
    class physics {
    public:
        /**
         * @brief Construct a new physics instance.
         */
        physics();

        /**
         * @brief Disabled copy constructor.
         */
        physics(const physics&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        physics(physics&& physics) noexcept;

        /**
         * @brief Destruct physics instance.
         */
        ~physics();

        /**
         * @brief Disabled copy assignment.
         */
        physics& operator=(const physics&) = delete;

        /**
         * @brief Disabled move assignment.
         */
        physics& operator=(physics&&) = delete;

    private:
        /** 
         * @brief Implementation specific data structure.
         */
        struct data;

        /**
         * @brief Implementation specific data pointer.
         */
        std::unique_ptr<data> m_data;
    };
}