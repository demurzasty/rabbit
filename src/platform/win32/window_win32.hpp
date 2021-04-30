
#pragma once

#include <rabbit/platform/window.hpp>
#include <rabbit/core/settings.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace rb {
    class window_win32 : public window {
    public:
        window_win32(settings& settings);

        ~window_win32();

		bool is_open() const override;

		window_handle native_handle() const override;

		void poll_events() override;

		vec2u size() const override;

		void set_title(const std::string& title) override;

		virtual std::string title() const override;

		void close();

	private:
		HWND _hwnd{ nullptr };
		bool _open{ true };
    };
}
