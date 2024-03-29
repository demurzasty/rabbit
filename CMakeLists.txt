cmake_minimum_required (VERSION 3.8.2)

project (rabbit)

add_subdirectory ("lib")

add_executable (b2h "src/tools/b2h.cpp")

file (GLOB_RECURSE BIN_SOURCE_FILES "src/*.spv")
foreach (BIN_SRC ${BIN_SOURCE_FILES})
	get_filename_component (FILE_DIRECTORY ${BIN_SRC} DIRECTORY)
	get_filename_component (FILE_NAME ${BIN_SRC} NAME)
	get_filename_component (FILE_EXT ${BIN_SRC} EXT)

	set (BIN_H "${FILE_DIRECTORY}/gen/${FILE_NAME}.h")

	string (REPLACE "." "_" VARIABLE_NAME ${FILE_NAME})

	add_custom_command (
		OUTPUT ${BIN_H}
		COMMAND b2h ${BIN_SRC} ${BIN_H} "${VARIABLE_NAME}" 
		DEPENDS ${BIN_SRC}
	)
	list (APPEND BIN_FILES ${BIN_H})
endforeach (BIN_SRC)
add_custom_target (binaries DEPENDS ${BIN_FILES})

set (SRC 
	"src/core/compressor.cpp"
	"src/core/reference.cpp"
	"src/core/stopwatch.cpp"
	"src/core/thread_pool.cpp"
	"src/graphics/font.cpp"
	"src/graphics/image.cpp"
	"src/graphics/painter.cpp"
	"src/graphics/rect_pack.cpp"
	"src/graphics/s3tc.cpp"
	"src/graphics/texture.cpp"
	"src/loaders/font_loader.cpp"
	"src/loaders/image_loader.cpp"
	"src/loaders/json_loader.cpp"
	"src/loaders/texture_loader.cpp"
	"src/network/enet/client.cpp"
	"src/network/enet/enet.cpp"
	"src/network/enet/server.cpp"
	"src/physics/body.cpp"
	"src/physics/physics.cpp"
	"src/physics/shape.cpp"
	"src/platform/input.cpp"
	"src/systems/canvas.cpp"
	"src/ui/ui.cpp"
)

if (WIN32)
	set (SRC ${SRC}
		"src/platform/win32/window_win32.cpp"
	)
endif ()

set (SRC ${SRC}
	"src/graphics/vulkan/renderer_vulkan.cpp"
	"src/graphics/vulkan/utils_vulkan.cpp"
)

add_library (rabbit STATIC ${SRC})
target_include_directories (rabbit PUBLIC "include")
target_compile_features (rabbit PUBLIC cxx_std_17)
target_link_libraries (rabbit PUBLIC json fmt entt gsl)
target_link_libraries (rabbit PRIVATE rgbcx miniz box2d enet stb vma volk)
target_compile_definitions (rabbit PRIVATE VMA_STATIC_VULKAN_FUNCTIONS)
add_dependencies (rabbit binaries)

if (WIN32)
	target_compile_definitions (rabbit PRIVATE VK_USE_PLATFORM_WIN32_KHR)
	target_link_libraries (rabbit PRIVATE ws2_32 winmm)
endif ()

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	add_subdirectory ("examples")
endif ()
