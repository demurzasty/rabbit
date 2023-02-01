cmake_minimum_required (VERSION 3.8.2)

project (rabbit)

add_subdirectory ("lib")

if (WIN32)
	set (SRC ${SRC}
		"src/platform/win32/window_win32.cpp"
	)
endif ()

add_library (rabbit STATIC ${SRC})
target_include_directories (rabbit PUBLIC "include")
target_compile_features (rabbit PUBLIC cxx_std_17)
target_link_libraries (rabbit PUBLIC entt)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	add_subdirectory ("examples")
endif ()
