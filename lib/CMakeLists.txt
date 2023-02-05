cmake_minimum_required (VERSION 3.8.2)

add_subdirectory ("box2d-2.4.1")

add_subdirectory ("enet-1.3.17")

add_subdirectory ("entt-3.11.1")

add_subdirectory ("fmt-9.1.0")

add_subdirectory ("gsl-4.0.0")

add_subdirectory ("json-3.11.2")

add_subdirectory ("stb")

add_subdirectory ("vma-3.0.1")

if (WIN32)
   set (VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

add_subdirectory ("volk-1.3.215")
