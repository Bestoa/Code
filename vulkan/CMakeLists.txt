cmake_minimum_required(VERSION 3.10)
project(VulkanExamples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)

# Find glm (header-only library)
find_package(glm QUIET)
if(NOT glm_FOUND)
    # Try to find glm in common locations
    find_path(GLM_INCLUDE_DIR glm/glm.hpp
        PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
    )
    if(GLM_INCLUDE_DIR)
        set(glm_INCLUDE_DIRS ${GLM_INCLUDE_DIR})
        message(STATUS "Found glm: ${GLM_INCLUDE_DIR}")
    else()
        message(WARNING "glm not found. Please install glm library.")
    endif()
endif()

include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(${GLFW3_INCLUDE_DIRS})
if(glm_INCLUDE_DIRS)
    include_directories(${glm_INCLUDE_DIRS})
endif()

add_executable(VulkanTriangle
    main.cpp
    triangle.vert
    triangle.frag
)

target_link_libraries(VulkanTriangle ${Vulkan_LIBRARIES} ${GLFW3_LIBRARIES})

add_executable(VulkanTriangleAnim
    main_anim.cpp
    triangle_anim.vert
    triangle_anim.frag
)

target_link_libraries(VulkanTriangleAnim ${Vulkan_LIBRARIES} ${GLFW3_LIBRARIES})

add_executable(VulkanTriangleAnimTexture
    main_anim_texture.cpp
    triangle_texture.vert
    triangle_texture.frag
)

target_link_libraries(VulkanTriangleAnimTexture ${Vulkan_LIBRARIES} ${GLFW3_LIBRARIES})

# Compile shaders
find_program(GLSLC_EXECUTABLE glslc)
if(GLSLC_EXECUTABLE)
    # Shaders for VulkanTriangle
    add_custom_command(
        OUTPUT triangle.vert.spv triangle.frag.spv
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/triangle.vert -o triangle.vert.spv
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/triangle.frag -o triangle.frag.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/triangle.vert ${CMAKE_SOURCE_DIR}/triangle.frag
        COMMENT "Compiling shaders for VulkanTriangle"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    add_custom_target(Shaders DEPENDS triangle.vert.spv triangle.frag.spv)
    add_dependencies(VulkanTriangle Shaders)
    
    # Shaders for VulkanTriangleAnim
    add_custom_command(
        OUTPUT triangle_anim.vert.spv triangle_anim.frag.spv
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/triangle_anim.vert -o triangle_anim.vert.spv
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/triangle_anim.frag -o triangle_anim.frag.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/triangle_anim.vert ${CMAKE_SOURCE_DIR}/triangle_anim.frag
        COMMENT "Compiling shaders for VulkanTriangleAnim"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    add_custom_target(ShadersAnim DEPENDS triangle_anim.vert.spv triangle_anim.frag.spv)
    add_dependencies(VulkanTriangleAnim ShadersAnim)
    
    # Shaders for VulkanTriangleAnimTexture
    add_custom_command(
        OUTPUT triangle_texture.vert.spv triangle_texture.frag.spv
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/triangle_texture.vert -o triangle_texture.vert.spv
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/triangle_texture.frag -o triangle_texture.frag.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/triangle_texture.vert ${CMAKE_SOURCE_DIR}/triangle_texture.frag
        COMMENT "Compiling shaders for VulkanTriangleAnimTexture"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    add_custom_target(ShadersAnimTexture DEPENDS triangle_texture.vert.spv triangle_texture.frag.spv)
    add_dependencies(VulkanTriangleAnimTexture ShadersAnimTexture)
    
    # Copy texture files to build directory
    add_custom_command(TARGET VulkanTriangleAnimTexture POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/checkerboard_1024x1024.png ${CMAKE_BINARY_DIR}/checkerboard_1024x1024.png
        COMMENT "Copying texture files to build directory"
    )
else()
    message(WARNING "glslc not found. Please compile shaders manually.")
endif()
