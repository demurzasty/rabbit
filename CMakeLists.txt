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

set (GLSLANG_VALIDATOR "${CMAKE_CURRENT_SOURCE_DIR}/bin/win32/glslangValidator.exe")

file (GLOB_RECURSE GLSL_SOURCE_FILES "data/shaders/*.vert" "data/shaders/*.frag")

foreach (GLSL_SRC ${GLSL_SOURCE_FILES})
	get_filename_component (FILE_NAME ${GLSL_SRC} NAME)
	get_filename_component (FILE_EXT ${GLSL_SRC} EXT)

	set (GENERATED_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/generated/include/rabbit/generated/shaders")
	set (SPIRV_H "${GENERATED_DIRECTORY}/${FILE_NAME}.spv.h")

	string (REPLACE "." "_" VARIABLE_NAME ${FILE_NAME})

	add_custom_command (
		OUTPUT ${SPIRV_H}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIRECTORY}
		# GLSL -> SPIR-V -> C Header
		COMMAND ${GLSLANG_VALIDATOR} -V ${GLSL_SRC} --vn "${VARIABLE_NAME}" -o ${SPIRV_H}
		DEPENDS ${GLSL_SRC}
	)
	list (APPEND SPIRV_BINARY_FILES ${SPIRV_H})
endforeach (GLSL_SRC)

add_custom_target (shaders DEPENDS ${SPIRV_BINARY_FILES})

set (SRC
	"src/analytics/profiler.cpp"
	"src/asset/asset_manager.cpp"
    "src/core/application.cpp"
    "src/core/builder.cpp"
    "src/core/system.cpp"
    "src/graphics/buffer.cpp"
    "src/graphics/graphics_device_provider.cpp"
    "src/graphics/material.cpp"
    "src/graphics/mesh.cpp"
    "src/graphics/resource_heap.cpp"
    "src/graphics/shader.cpp"
    "src/graphics/texture.cpp"
    "src/loaders/material_loader.cpp"
    "src/loaders/mesh_loader.cpp"
    "src/loaders/texture_loader.cpp"
    "src/platform/window_provider.cpp"
	"src/systems/render_system.cpp"
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
        "src/graphics/vulkan/material_vulkan.cpp"
        "src/graphics/vulkan/mesh_vulkan.cpp"
        "src/graphics/vulkan/resource_heap_vulkan.cpp"
        "src/graphics/vulkan/shader_vulkan.cpp"
        "src/graphics/vulkan/texture_vulkan.cpp"
        "src/graphics/vulkan/utils_vulkan.cpp"
	)
endif ()

add_library (rabbit STATIC ${SRC})
target_include_directories (rabbit PUBLIC "include" PRIVATE "generated/include")
target_compile_features (rabbit PUBLIC cxx_std_17)
target_link_libraries (rabbit PUBLIC stb fmt gsl json entt)
add_dependencies (rabbit shaders)

if (MSVC)
	target_compile_definitions (rabbit PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif ()

if (RB_VULKAN)
	target_link_libraries (rabbit PUBLIC vma volk)
	target_compile_definitions (rabbit PRIVATE VMA_STATIC_VULKAN_FUNCTIONS VK_USE_PLATFORM_WIN32_KHR)
endif ()

target_compile_definitions (rabbit PUBLIC RB_WINDOWS=$<BOOL:${RB_WINDOWS}>)
target_compile_definitions (rabbit PUBLIC RB_LINUX=$<BOOL:${RB_LINUX}>)
target_compile_definitions (rabbit PUBLIC RB_MACOS=$<BOOL:${RB_MACOS}>)

target_compile_definitions (rabbit PUBLIC RB_VULKAN=$<BOOL:${RB_VULKAN}>)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	add_subdirectory ("example")
endif ()
