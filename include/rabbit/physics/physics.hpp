#pragma once 

#include "../core/handle.hpp"
#include "../math/vec2.hpp"

#include <memory>

namespace rb {
    /**
     * @brief Type of a body.
     */
    enum class body_type : unsigned char {
        /**
         * @brief Zero mass, zero velocity, may be manually moved.
         */
        type_static,

        /**
         * @brief Positive mass, non-zero velocity determined by forces, moved by solver
         */
        type_kinematic,

        /**
         * @brief Zero mass, non-zero velocity set by user, moved by solver.
         */
        type_dynamic
    };

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

        /**
         * @brief Creates a circle shape.
         */
        handle create_circle_shape(float radius);

        /**
         * @brief Creates a box shape.
         */
        handle create_box_shape(const vec2& extents);

        /**
         * @brief Destroy the shape.
         * 
         * @warning Attempting to destroy a shape that is invalid 
                    or being destroyed results in undefined behavior.
         *
         * @param id Shape handle.
         */
        void destroy_shape(handle id);

        /**
         * @brief Creates a physics body.
         * 
         * @param type Type of a body.
         */
        handle create_body(body_type type);

        /**
         * @brief Destroy the physics body.
         * 
         * @warning Attempting to destroy a body that is invalid 
                    or being destroyed results in undefined behavior.
         *
         * @param id Body handle.
         */
        void destroy_body(handle id);

        /**
         * @brief Attach shape to the body.
         *
         * @warning Attempting to set shape to a body that is invalid
                    or being destroyed results in undefined behavior.
         *
         * @param id Body handle.
         * @param shape_id Id of shape to attach.
         */
        void set_body_shape(handle id, handle shape_id);

        /**
         * @brief Set position of the body.
         *
         * @warning Attempting to set position of the body that is invalid
                    or being destroyed results in undefined behavior.
         *
         * @param id Body handle.
         * @param position New position of the body.
         */
        void set_body_position(handle id, const vec2& position);

        /**
         * @brief Set rotation of the body.
         *
         * @warning Attempting to set rotation of the body that is invalid
                    or being destroyed results in undefined behavior.
         *
         * @param id Body handle.
         * @param rotation New rotation in radians of the body.
         */
        void set_body_rotation(handle id, float rotation);

        /**
         * @brief Get current body position.
         *
         * @warning Attempting to set shape to a body that is invalid
                    or being destroyed results in undefined behavior.
         *
         * @param id Body handle.
         * 
         * @return Position of the body.
         */
        vec2 get_body_position(handle id) const;

        /**
         * @brief Get current body rotation.
         *
         * @warning Attempting to set shape to a body that is invalid
                    or being destroyed results in undefined behavior.
         *
         * @param id Body handle.
         *
         * @return Rotation in radians of the body.
         */
        float get_body_rotation(handle id) const;

        /**
         * @brief Simulate world. This performs collision detection, integration,
	     *        and constraint solution.
         * 
	     * @param time_step The amount of time to simulate, this should not vary.
         */
        void simulate(float time_step);

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