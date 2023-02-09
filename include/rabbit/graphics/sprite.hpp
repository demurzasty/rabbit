#pragma once 

#include "../core/reference.hpp"
#include "renderer.hpp"
#include "texture.hpp"

namespace rb {
    /**
     * @brief Drawable representation of a texture component.
     *        This is an object-oriented wrapper of sprite
     *        handle created by a renderer.
     */
    class sprite : public reference {
    public:
        /**
         * @brief Disable default construction.
         */
        sprite() = delete;

        /**
         * @brief Construct a new sprite.
         *
         * @param renderer The renderer this sprite belongs to.
         */
        sprite(renderer& renderer);

        /**
         * @brief Construct a new sprite.
         *
         * @param renderer The renderer this sprite belongs to.
         * @param texture Initial texture to set.
         */
        sprite(renderer& renderer, ref<texture> texture);

        /**
         * @brief Disabled copy constructor.
         */
        sprite(const sprite&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        sprite(sprite&& sprite) noexcept;

        /**
         * @brief Destructor of the sprite.
         */
        ~sprite();

        /**
         * @brief Disabled copy assignment.
         */
        sprite& operator=(const sprite&) = delete;

        /**
         * @brief Disabled move assignment.
         */
        sprite& operator=(sprite&&) = delete;

        /**
         * @brief Cast sprite directly to sprite handle.
         *
         * @return Texture handle.
         */
        operator handle() const;

        /** 
         * @brief Set transform.
         * 
         * @param transform Sprite transform.
         */
        void set_transform(const mat2x3& transform);

        /**
         * @brief Get transform.
         * 
         * @return Sprite transform.
         */
        const mat2x3& transform() const;

        /**
         * @brief Set texture handle.
         *
         * @param texture Texture handle to set. Can be null.
         */
        void set_texture(ref<texture> texture);

        /**
         * @brief Get texture handle.
         * 
         * @return Texture handle.Can be null.
         */
        [[nodiscard]] const ref<texture>& texture() const;

        /**
         * @brief Get the handle of the sprite.
         *
         * @return Handle of the sprite.
         */
        [[nodiscard]] handle id() const;

    private:
        /**
         * @biref Renderer to which this sprite is attached.
         */
        renderer& m_renderer;

        /**
         * @brief Sprite id created by a renderer.
         */
        handle m_id = null;

        /**
         * @brief Keep a reference to texture.
         */
        ref<rb::texture> m_texture = nullptr;
    };
}
