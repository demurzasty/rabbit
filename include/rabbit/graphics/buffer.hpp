#pragma once

#include "../core/config.hpp"

#include <cstddef>

// TODO: Update buffer with offset and size arguments.

namespace rb {
    /**
     * @brief Hardware buffer type.
     */
    enum class buffer_type {
        vertex,
        index,
        uniform
    };

    /**
     * @brief Hardware buffer descriptor structure.
     */
    struct buffer_desc {
        buffer_type type{ buffer_type::vertex };
        const void* data{ nullptr };
        std::size_t size{ 0 };
        std::size_t stride{ 0 };
    };

    /**
     * @brief Hardware buffer interface.
     *
     * @see graphics_device::create_buffer
     */
    class buffer {
    public:
        /**
         * @brief Default virtual destructor.
         */
        virtual ~buffer() = default;

        /**
         * @brief Update conent of the buffer with provided data.
         *
         * @note Provided data must be size of the buffer.
         */
        virtual void update(const void* data) = 0;

        /**
         * @brief Returns buffer type.
         */
        buffer_type type() const;

        /**
         * @brief Returns buffer size.
         */
        std::size_t size() const;

        /**
         * @brief Returns buffer stride.
         */
        std::size_t stride() const;

        /**
         * @brief Returns buffer element count. It is basically size() / stride().
         */
        std::size_t count() const;

    protected:
        buffer(const buffer_desc& desc);

    private:
        const buffer_type _type;
        const std::size_t _size;
        const std::size_t _stride;
    };
}