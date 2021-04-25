#pragma once

namespace rb {
    enum class topology {
		/**
		 * @brief The data is ordered as a sequence of line segments.
		 *	      Each line segment is described by two new vertices.
		 *	      The count may be any positive integer.
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
}
