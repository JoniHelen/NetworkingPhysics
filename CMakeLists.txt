cmake_minimum_required (VERSION 3.26)

include(FetchContent)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

FetchContent_Declare (
	box2d
	GIT_REPOSITORY  https://github.com/erincatto/box2d.git
	GIT_TAG			411acc32eb6d4f2e96fc70ddbdf01fe5f9b16230
)

FetchContent_Declare (
	asio
	GIT_REPOSITORY  https://github.com/chriskohlhoff/asio.git
	GIT_TAG			ed5db1b50136bace796062c1a6eab0df9a74f8fa
)

FetchContent_MakeAvailable (box2d asio)

project ("NetworkingPhysics")

add_executable (NetworkingPhysics 
	"NetworkingPhysics.cpp"
	"NetworkingPhysics.h"
	"imgui_impl_glfw.cpp"
	"imgui_impl_glfw.h"
	"imgui_impl_opengl3.cpp"
	"imgui_impl_opengl3.h"
)

target_include_directories (NetworkingPhysics 
	PRIVATE 
	"${box2d_SOURCE_DIR}/include"
	"${box2d_SOURCE_DIR}/extern/glfw/include"
	"${box2d_SOURCE_DIR}/extern/glad/include"
	"${box2d_SOURCE_DIR}/extern/imgui/include"
	"linmath"
	"${asio_SOURCE_DIR}/asio/include"
)

target_link_libraries (NetworkingPhysics PRIVATE box2d glfw glad imgui)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NetworkingPhysics PROPERTY CXX_STANDARD 20)
endif()