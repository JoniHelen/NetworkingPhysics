cmake_minimum_required (VERSION 3.26)

include(FetchContent)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

FetchContent_Declare(
	box2d
	GIT_REPOSITORY  https://github.com/erincatto/box2d.git
	GIT_TAG			411acc32eb6d4f2e96fc70ddbdf01fe5f9b16230
)

FetchContent_Declare(
	asio
	GIT_REPOSITORY  https://github.com/chriskohlhoff/asio.git
	GIT_TAG			ed5db1b50136bace796062c1a6eab0df9a74f8fa
)

FetchContent_MakeAvailable(box2d asio)

project(NetworkingPhysics)

add_executable(NetworkingPhysics
	src/NetworkingPhysics.cpp
	include/NetworkingPhysics.h
	src/imgui_impl_glfw.cpp
	include/imgui_impl_glfw.h
	src/imgui_impl_opengl3.cpp
	include/imgui_impl_opengl3.h
	src/pch.cpp
	include/pch.h
	src/Netcode.cpp
	include/Netcode.h
	src/main.cpp
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NetworkingPhysics PROPERTY CXX_STANDARD 20)
endif()

target_precompile_headers(NetworkingPhysics PRIVATE include/pch.h)

target_include_directories(NetworkingPhysics 
	PRIVATE 
	"${box2d_SOURCE_DIR}/include"
	"${box2d_SOURCE_DIR}/extern/glfw/include"
	"${box2d_SOURCE_DIR}/extern/glad/include"
	"${box2d_SOURCE_DIR}/extern/imgui/include"
	"linmath"
	"include"
	"${asio_SOURCE_DIR}/asio/include"
)

target_link_libraries(NetworkingPhysics PRIVATE box2d glfw glad imgui ws2_32)

add_custom_command(TARGET NetworkingPhysics POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(TARGET NetworkingPhysics POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/LaunchServer.bat ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(TARGET NetworkingPhysics POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/LaunchClient.bat ${CMAKE_CURRENT_BINARY_DIR}
)