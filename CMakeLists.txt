cmake_minimum_required(VERSION 3.0.0)
project(vulkan-engine VERSION 0.1.0)

function(add_shader TARGET SHADER)
    find_program(GLSLC glslc)

    set(current-shader-path ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/${SHADER})
    set(current-output-path ${CMAKE_BINARY_DIR}/shaders/${SHADER}.spv)

    # Add a custom command to compile GLSL to SPIR-V.
    get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
    file(MAKE_DIRECTORY ${current-output-dir})

    add_custom_command(
           OUTPUT ${current-output-path}
           COMMAND ${GLSLC} -o ${current-output-path} ${current-shader-path}
           DEPENDS ${current-shader-path}
           IMPLICIT_DEPENDS CXX ${current-shader-path}
           VERBATIM)

    # Make sure our build depends on this output.
    set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET} PRIVATE ${current-output-path})
endfunction(add_shader)

set(SOURCES
    src/vk_mem_alloc.h

    src/main.cpp
    src/ve_window.hpp
    src/ve_window.cpp
    src/app.hpp
    src/app.cpp
    src/ve_pipeline.hpp
    src/ve_pipeline.cpp
    src/ve_device.hpp
    src/ve_device.cpp
    src/ve_swapchain.hpp
    src/ve_swapchain.cpp
    src/ve_model.hpp
    src/ve_model.cpp
    src/ve_game_object.hpp
    src/ve_game_object.cpp
    src/ve_renderer.hpp
    src/ve_renderer.cpp
    src/ve_camera.hpp
    src/ve_camera.cpp
    src/ve_input.hpp
    src/ve_input.cpp
    src/ve_timer.hpp
    src/ve_timer.cpp

    src/simple_render_system.hpp
    src/simple_render_system.cpp
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(PROJECT_SOURCE_DIR /src)

add_executable(vulkan-engine ${SOURCES})

target_include_directories(vulkan-engine PRIVATE ${PROJECT_SOURCE_DIR})

add_shader(vulkan-engine simple.vert)
add_shader(vulkan-engine simple.frag)

find_package(Vulkan REQUIRED)
target_link_libraries(vulkan-engine Vulkan::Vulkan)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
target_include_directories(vulkan-engine PRIVATE ${PROJECT_SOURCE_DIR}/glfw/include)
target_link_libraries(vulkan-engine glfw ${GLFW_LIBRARIES})
add_dependencies(vulkan-engine glfw)

add_subdirectory(glm)
target_link_libraries(vulkan-engine glm::glm)

# add_subdirectory(assimp)
# SET(ASSIMP_BUILD_TESTS OFF)
# target_link_libraries(vulkan-engine Assimp)