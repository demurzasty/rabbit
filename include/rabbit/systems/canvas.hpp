#pragma once 

#include "../graphics/painter.hpp"
#include "../entity/entity.hpp"

namespace rb {
	/**
	 * @brief Built-in canvas system.
	 */
	class canvas {
	public:
		/**
		 * @brief Construct a new canvas.
		 * 
		 * @param painter Reference to the painter.
		 */
		canvas(painter& painter);

		/**
		 * @brief Run the canvas system.
		 * 
		 * @param registry Reference to the registry.
		 * @param time_step Delta time between frames.
		 */
		void process(registry& registry, float time_step);

	private:
		painter& m_painter;
	};
}