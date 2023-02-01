#pragma once 

namespace rb {
    /**
     * @brief Utility structure for manipulating RGBA colors.
     */
    struct color {
        /**
         * @brief Make black predefined color.
         * 
         * @return Black predefined color.
         */
        static constexpr color black() { return { 0, 0, 0, 255 }; }

        /**
         * @brief Make white predefined color.
         * 
         * @return White predefined color.
         */
        static constexpr color white() { return { 255, 255, 255, 255 }; }

        /**
         * @brief Make red predefined color.
         * 
         * @return Red predefined color.
         */
        static constexpr color red() { return { 255, 0, 0, 255 }; }

        /**
         * @brief Make green predefined color.
         * 
         * @return Green predefined color.
         */
        static constexpr color green() { return { 0, 255, 0, 255 }; }

        /**
         * @brief Make blue predefined color.
         * 
         * @return Blue predefined color.
         */
        static constexpr color blue() { return { 0, 0, 255, 255 }; }

        /**
         * @brief Make transparent (black) predefined color.
         * 
         * @return Transparent (black) predefined color.
         */
        static constexpr color transparent() { return { 0, 0, 0, 0 }; }

        /**
         * @brief Make yellow predefined color.
         * 
         * @return Yellow predefined color.
         */
        static constexpr color yellow() { return { 255, 255, 0, 255 }; }

        /**
         * @brief Make magenta predefined color.
         * 
         * @return Magenta predefined color.
         */
        static constexpr color magenta() { return { 255, 0, 255, 255 }; }

        /**
         * @brief Make cyan predefined color.
         * 
         * @return Cyan predefined color.
         */
        static constexpr color cyan() { return { 0, 255, 255, 255 }; }

        /**
         * @brief Make cornflower blue predefined color.
         * 
         * @return Cornflower blue predefined color.
         */
        static constexpr color cornflower_blue() { return { 100, 149, 237, 255 }; }

        /**
         * @brief Retrieve the color as a 32-bit unsigned integer.
         * 
         * @return Color represented as a 32-bit unsigned integer.
         */
        constexpr unsigned int to_integer() const {
            return (unsigned int)((r << 24) | (g << 16) | (b << 8) | a);
        }

        /**
         * @brief Red component.
         */
        unsigned char r;

        /**
         * @brief Green component.
         */
        unsigned char g;

        /**
         * @brief Blue component.
         */
        unsigned char b;

        /**
         * @brief Alpha (opacity) component.
         */
        unsigned char a;
    };
}
