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
         * @brief Destruct physics instance.
         */
        ~physics();

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