#pragma once 

#include "physics.hpp"
#include "shape.hpp"
#include "../core/reference.hpp"

namespace rb {
    /**
     * @brief Physics body.
     */
    class body : public reference {
    public:
        /**
         * @brief Disable default construction.
         */
        body() = delete;

        /**
         * @brief Construct a new body.
         *
         * @param physics The physics this body belongs to.
         * @param type Type of a body.
         */
        body(physics& physics, body_type type);

        /**
         * @brief Disabled copy constructor.
         */
        body(const body&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        body(body&& body) noexcept;

        /**
         * @brief Destructor of the body.
         */
        ~body();

        /**
         * @brief Disabled copy assignment.
         */
        body& operator=(const body&) = delete;

        /**
         * @brief Disabled move assignment.
         */
        body& operator=(body&&) = delete;

        /**
         * @brief Cast body directly to body handle.
         *
         * @return Body handle.
         */
        operator handle() const;

        /**
         * @brief Set shape of the body.
         * 
         * @param shape Shape of the body.
         */
        void set_shape(ref<shape> shape);

        /**
         * @brief Set position of the body.
         * 
         * @param position New position of the body.
         */
        void set_position(const vec2& position) const;

        /**
         * @brief Set rotation of the body.
         *
         * @param rotation New rotation in radians of the body.
         */
        void set_rotation(float rotation) const;

        /**
         * @brief Get position of the body.
         * 
         * @return Position of the body in screen coordinates.
         */
        vec2 position() const;

        /**
         * @brief Get rotation of the body.
         *
         * @return Rotation of the body in radians.
         */
        float rotation() const;

    private:
        /**
         * @biref Physics to which this body is attached.
         */
        physics& m_physics;

        /**
         * @brief Body id created by a physics.
         */
        handle m_id = null;

        /**
         * @brief Attached shape.
         */
        ref<shape> m_shape;
    };
}