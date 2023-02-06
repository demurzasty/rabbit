#pragma once 

#include "physics.hpp"
#include "../core/reference.hpp"

namespace rb {
    /**
     * @brief Physics shape.
     */
    class shape : public reference {
    public:
        /**
         * @brief Disable default construction.
         */
        shape() = delete;

        /**
         * @brief Disabled copy constructor.
         */
        shape(const shape&) = delete;

        /**
         * @brief Disabled move constructor.
         */
        shape(shape&& shape) noexcept;

        /**
         * @brief Destructor of the shape.
         */
        virtual ~shape();

        /**
         * @brief Disabled copy assignment.
         */
        shape& operator=(const shape&) = delete;

        /**
         * @brief Disabled move assignment.
         */
        shape& operator=(shape&&) = delete;

        /**
         * @brief Cast shape directly to shape handle.
         *
         * @return Body handle.
         */
        operator handle() const;

    protected:
        /**
         * @brief Construct a new shape.
         *
         * @param physics The physics this shape belongs to.
         * @param id Handle of a shape.
         */
        shape(physics& physics, handle id);

        /**
         * @biref Physics to which this shape is attached.
         */
        physics& m_physics;

        /**
         * @brief Body id created by a physics.
         */
        handle m_id = null;
    };

    /**
     * @brief Circle physics shape.
     */
    class circle_shape : public shape {
    public:
        /**
         * @brief Creates a circle shape.
         */
        circle_shape(physics& physics, float radius);

        /**
         * @brief Enabled move constructor.
         */
        circle_shape(circle_shape&& shape) noexcept;
    };

    /**
     * @brief Box physics shape.
     */
    class box_shape : public shape {
    public:
        /**
         * @brief Creates a box shape.
         */
        box_shape(physics& physics, const vec2& extents);

        /**
         * @brief Enabled move constructor.
         */
        box_shape(box_shape&& shape) noexcept;
    };
}