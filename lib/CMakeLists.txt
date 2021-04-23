cmake_minimum_required (VERSION 3.8.2)

add_subdirectory ("entt-3.7.1")

add_subdirectory ("gsl-3.1.0")

add_subdirectory ("json-3.9.1")

add_subdirectory ("stb")

if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

add_subdirectory ("volk-1.2.170")

add_subdirectory ("vma-2.3.0")
