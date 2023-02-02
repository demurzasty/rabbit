#pragma once 

#include "../math/vec2.hpp"
#include "../math/vec4.hpp"

#include <memory>

namespace rb {
    /**
     * @brief Useful for e.g. packing rectangular textures into an atlas.
     */
    class rect_pack {
    public:
        /**
         * @brief Construct new rect pack.
         * 
         * @param size Size of rectangle.
         */
        rect_pack(const uvec2& size);

        /**
         * @brief Disable copy constructor.
         */
        rect_pack(const rect_pack&) = delete;

        /**
         * @brief Move constructor.
         */
        rect_pack(rect_pack&&) noexcept;

        /**
         * @brief Disable copy assignment.
         */
        rect_pack& operator=(const rect_pack&) = delete;

        /**
         * @brief Move assignment.
         */
        rect_pack& operator=(rect_pack&&) noexcept;

        /**
         * @brief Due to impementation defined member data
         *        we need to provide custom destructor.
         */
        ~rect_pack();

        /**
         * @brief Assign packed locations to rectangle.
         * 
         * @return Packed rectangle.
         */
        [[nodiscard]] ivec4 pack(const uvec2& size);

    private:
        /**
         * @brief Implementation defined data structure.
         */
        struct data;

        /**
         * @brief Implementation defined data pointer.
         */
        std::unique_ptr<data> m_data;
    };
}
