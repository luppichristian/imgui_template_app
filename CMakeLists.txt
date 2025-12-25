cmake_minimum_required(VERSION 3.19)
set(TITLE imgui_template_app)

# Project
project(${TITLE} LANGUAGES CXX)

# Using FetchContent for third party libraries
include(FetchContent)

# Enable static runtime library
set(FORCE_STATIC_VCRT ON)

# Enable UTF8
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:char8_t-")
endif()

# Fetch SDL3
set(SDL_SHARED Off)
set(SDL_STATIC On)
FetchContent_Declare(sdl3 GIT_REPOSITORY https://github.com/libsdl-org/SDL GIT_TAG release-3.2.22 OVERRIDE_FIND_PACKAGE)
FetchContent_GetProperties(sdl3 SOURCE_DIR SDL3_SOURCE_DIR)
FetchContent_MakeAvailable(sdl3)

# Fetch ImGui
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui GIT_TAG 3109131a882daec56a530aff540416983c240443 OVERRIDE_FIND_PACKAGE)
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui SOURCE_DIR IMGUI_SOURCE_DIR)

# Setup ImGui target
add_library(ImGui STATIC
  "${IMGUI_SOURCE_DIR}/imgui_demo.cpp"
  "${IMGUI_SOURCE_DIR}/imgui.cpp"
  "${IMGUI_SOURCE_DIR}/imgui_draw.cpp"
  "${IMGUI_SOURCE_DIR}/imgui_tables.cpp"
  "${IMGUI_SOURCE_DIR}/imgui_widgets.cpp"
  "${IMGUI_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
  "${IMGUI_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.cpp"
)
target_include_directories( ImGui PUBLIC
 ${IMGUI_SOURCE_DIR}
 ${IMGUI_SOURCE_DIR}/backends
 ${SDL3_SOURCE_DIR}/include
)

# Create main executable
add_executable(${TITLE})

# Setup source code
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")
target_sources(${TITLE} PRIVATE ${SRC})

# Setup build properties for the target
set_property(TARGET ${TITLE} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${TITLE} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${TITLE} PROPERTY CXX_EXTENSIONS ON)
set_target_properties(${TITLE} PROPERTIES UNITY_BUILD On)
set_target_properties(${TITLE} PROPERTIES UNITY_BUILD_MODE BATCH)
set_target_properties(${TITLE} PROPERTIES UNITY_BUILD_BATCH_SIZE 0)
set_target_properties(${TITLE} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Include directories
target_include_directories(${TITLE} PRIVATE "src/")

# Linker options
if(WIN32)
  target_link_options(${TITLE} PRIVATE "/SUBSYSTEM:WINDOWS")
endif()

# Libraries
target_link_libraries(${TITLE} PRIVATE SDL3-static ImGui)