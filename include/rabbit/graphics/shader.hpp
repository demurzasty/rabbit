#pragma once

#include "mesh.hpp"
#include "texture.hpp"
#include "../core/span.hpp"

#include <vector>
#include <memory>
#include <cstdint>

namespace rb {
    enum class topology {
        undefined,

        /**
         * @brief The data is ordered as a sequence of line segments.
         *        Each line segment is described by two new vertices.
         *        The count may be any positive integer.
         */
        lines,

        /**
         * @brief The data is ordered as a sequence of line segments.
         *        Each line segment is described by one new vertex and the last vertex from the previous line segment.
         *        The count may be any positive integer.
         */
        line_strip,

        /**
         * @brief The data is ordered as a sequence of triangles.
         *        Each triangle is described by three new vertices.
         *        Back-face culling is affected by the current winding-order render state.
         */
        triangles,

        /**
         * @brief The data is ordered as a sequence of triangles.
         *        Each triangle is described by two new vertices and one vertex from the previous triangle.
         *        The back-face culling flag is flipped automatically on even-numbered triangles.
         */
        triangle_strip
    };

    /**
     * @brief Defines which faces should be culled.
     */
    enum class cull_mode {
        undefined,

        /**
         * @brief Do not cull faces.
         */
        none,

        /**
         * @brief Cull front faces.
         */
        front,

        /**
         * @brief Cull back faces.
         */
        back,
    };

    enum class polygon_mode {
        undefined,
        fill,
        line,
        point
    };

    /**
     * @brief Defines winding orders that may be used to identify faces as front faces.
     */
    enum class front_face {
        undefined,

        /**
         * @brief Treat counterclockwise faces as front faces.
         */
        counter_clockwise,

        /**
         * @brief Treat clockwise faces as front faces.
         */
        clockwise
    };

    /**
     * @brief Defines comparison operators that can be chosen for alpha, stencil, or depth-buffer tests.
     */
    enum class compare_operator {
        undefined,

        /**
         * @brief Always pass the test.
         */
        always,

        /**
         * @brief Always fail the test.
         */
        never,

        /**
         * @brief Accept the new pixel if its value is less than the value of the current pixel.
         */
        less,

        /**
         * @brief Accept the new pixel if its value is less than or equal to the value of the current pixel.
         */
        less_equal,

        /**
         * @brief Accept the new pixel if its value is equal to the value of the current pixel.
         */
        equal,

        /**
         * @brief Accept the new pixel if its value is greater than or equal to the value of the current pixel.
         */
        greater_equal,

        /**
         * @brief Accept the new pixel if its value is greater than the value of the current pixel.
         */
        greater,

        /**
         * @brief Accept the new pixel if its value does not equal the value of the current pixel.
         */
        not_equal
    };

    enum class shader_binding_type {
        undefined,
        uniform_buffer,
        texture
    };

    struct shader_stage_flags {
        using type = std::uint32_t;

        static constexpr type vertex{ 1 };
        static constexpr type fragment{ 2 };
    };

    /**
     * @brief Directly shader binding description used in shader.
     */
    struct shader_binding {
        shader_binding_type type{ shader_binding_type::uniform_buffer };
        shader_stage_flags::type stage_flags{ 0 };
        std::uint32_t slot{ 0 };
        std::uint32_t array_size{ 1 };
    };

    /**
     * @brief Shader descriptor structure.
     */
    struct shader_desc {
        vertex_layout vertex_layout;
        span<const std::uint32_t> vertex_bytecode;
        span<const std::uint32_t> fragment_bytecode;
        std::vector<shader_binding> bindings;
        std::shared_ptr<texture> render_target;

        // Input assembly
        topology topology{ topology::triangles };

        // Rasterizer state
        polygon_mode polygon_mode{ polygon_mode::fill };
        cull_mode cull_mode{ cull_mode::back };
        front_face front_face{ front_face::counter_clockwise };

        // Depth-Stencil state
        bool depth_test_enable{ true };
        bool depth_write_enable{ true };
        bool stencil_test_enable{ false };
        compare_operator depth_compare_operator{ compare_operator::less };
    };

    class shader {
    public:
        /**
         * @brief Default virtual destructor.
         */
        virtual ~shader() = default;

        /**
         * @brief Returns vertex layout descriptor.
         */
        const vertex_layout& vertex_layout() const;

        /**
         * @brief Returns bindings.
         */
        const std::vector<shader_binding>& bindings() const;

        /**
         * @brief Returns associated render target.
         *
         * @return Render target, or nullptr if not provided.
         */
        const std::shared_ptr<texture>& render_target() const;

        /**
         * @brief Returns topology.
         */
        topology topology() const;

        /**
         * @brief Returns polygon mode.
         */
        polygon_mode polygon_mode() const;

        /**
         * @brief Returns cull mode.
         */
        cull_mode cull_mode() const;

        /**
         * @brief Returns front face.
         */
        front_face front_face() const;

        /**
         * @brief Tell whether depth testing is enable.
         */
        bool is_depth_test() const;

        /**
         * @brief Tell whether depth writing is enable.
         */
        bool is_depth_write() const;

        /**
         * @brief Tell whether stencil testing is enable.
         */
        bool is_stencil_test() const;

        /**
         * @brief Returns depth compare operator.
         */
        compare_operator depth_compare_operator() const;

    protected:
        shader(const shader_desc& desc);

    private:
        const rb::vertex_layout _vertex_layout;
        const std::vector<shader_binding> _bindings;
        const std::shared_ptr<texture> _render_target;
        const rb::topology _topology;
        const rb::polygon_mode _polygon_mode;
        const rb::cull_mode _cull_mode;
        const rb::front_face _front_face;
        const bool _depth_test_enable;
        const bool _depth_write_enable;
        const bool _stencil_test_enable;
        const compare_operator _depth_compare_operator;
    };
}
