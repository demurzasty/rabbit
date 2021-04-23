cmake_minimum_required (VERSION 3.8.2)

project (rabbit)

if (WIN32)
	set (RB_WINDOWS TRUE)
elseif (UNIX AND NOT APPLE)
	set (RB_LINUX TRUE)
elseif (APPLE)
	set (RB_OSX TRUE)
endif()

set (RB_VULKAN TRUE)

add_subdirectory ("lib")

set (SRC
	"src/platform/window_manager.cpp"
	"src/graphics/buffer.cpp"
	"src/graphics/graphics_device_manager.cpp"
	"src/graphics/graphics_device.cpp"
)

if (RB_WINDOWS)
	set (SRC ${SRC}
		"src/platform/win32/window_win32.cpp"
	)
endif ()

if (RB_VULKAN)
	set (SRC ${SRC}
		"src/graphics/vulkan/buffer_vulkan.cpp"
		"src/graphics/vulkan/command_buffer_vulkan.cpp"
		"src/graphics/vulkan/graphics_device_vulkan.cpp"
		"src/graphics/vulkan/utils_vulkan.cpp"
	)
endif ()

add_library (rabbit STATIC ${SRC})
target_include_directories (rabbit PUBLIC "include")
target_compile_features (rabbit PUBLIC cxx_std_17)
target_link_libraries (rabbit PUBLIC vma volk stb gsl json entt)
target_compile_definitions (rabbit PRIVATE VMA_STATIC_VULKAN_FUNCTIONS NOMINMAX VK_USE_PLATFORM_WIN32_KHR)

target_compile_definitions (rabbit PUBLIC RB_WINDOWS=$<BOOL:${RB_WINDOWS}>)
target_compile_definitions (rabbit PUBLIC RB_LINUX=$<BOOL:${RB_LINUX}>)
target_compile_definitions (rabbit PUBLIC RB_MACOS=$<BOOL:${RB_MACOS}>)

target_compile_definitions (rabbit PUBLIC RB_VULKAN=$<BOOL:${RB_VULKAN}>)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	add_subdirectory ("example")
endif ()
