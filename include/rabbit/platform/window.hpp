#pragma once

#include "window_handle.hpp"
#include "../math/vec2.hpp"
#include "../core/non_copyable.hpp"

#include <string>

namespace rb {
    /**
     * @brief Window base class.
     *
     *        Can be provided by window_service_provider through injector.
     *
     * @see window_provider
     */
    class window : public non_copyable {
    public:
		/**
		 * @brief Default virtual destructor.
		 */
		virtual ~window() = default;

		/**
		 * @brief Tell whether window is open.
		 *
		 * @return True if window is open.
		 */
		virtual bool is_open() const = 0;

		/**
		 * @brief Returns native window handle.
		 *
		 * @return Native window handle.
		 */
		virtual window_handle native_handle() const = 0;

		/**
		 * @brief Poll window events.
		 *
		 * @return True if event queue was not empty.
		 */
		virtual void poll_events() = 0;

		/**
		 * @brief Returns window size.
		 *
		 * @return Window size.
		 */
		virtual vec2u size() const = 0;

		/**
		 * @brief Sets window title.
		 */
		virtual void set_title(const std::string& title) = 0;

		/**
		 * @brief Retrieve window title.
		 *
		 * @return Window title.
		 */
		virtual std::string title() const = 0;
    };
}
