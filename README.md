# ImGui Template App

A cross-platform C++ application template using Dear ImGui and SDL3.

## Description

This project provides a ready-to-use template for building graphical applications with Dear ImGui. It includes a complete build setup using CMake and automatically fetches dependencies (ImGui and SDL3) during configuration.

SDL_GPU was chosen for its excellent cross-platform graphics support, providing a modern GPU API that works seamlessly across Windows, Linux, and macOS.

## Prerequisites

- **CMake** (version 3.14 or higher)
- **C++ Compiler** with C++17 support
  - Windows: MSVC, MinGW, or Clang
  - Linux: GCC or Clang
  - macOS: Clang (Xcode Command Line Tools)
- **Ninja** (optional, but recommended for faster builds)

## Dependencies

The following dependencies are automatically fetched and built by CMake:
- [Dear ImGui](https://github.com/ocornut/imgui) (docking branch) - Immediate mode GUI library with docking and viewport support
- [SDL3](https://github.com/libsdl-org/SDL) - Cross-platform multimedia library

## Building

### Windows

```powershell
# Configure the project
cmake -B build -G Ninja

# Build the project
cmake --build build

# Or build in release mode
cmake --build build --config Release
```

### Linux / macOS

```bash
# Configure the project
cmake -B build

# Build the project
cmake --build build

# Or build in release mode
cmake --build build --config Release
```

## Running

After building, run the executable:

**Windows:**
```powershell
.\build\imgui_template_app.exe
```

**Linux / macOS:**
```bash
./build/imgui_template_app
```

## Project Structure

```
imgui_template_app/
├── CMakeLists.txt          # CMake build configuration
├── imgui.ini               # ImGui layout configuration
├── src/                    # Source code
│   ├── app.cpp             # Application implementation
│   ├── app.h               # Application header
│   ├── user.cpp            # User code implementation
│   └── user.h              # User code header
└── build/                  # Build output directory
    └── _deps/              # Fetched dependencies
```

## Customization

### Adding Your Code

The template is organized to separate application framework from user code:

- **app.cpp / app.h**: Core application setup, window management, and main loop
- **user.cpp / user.h**: Your custom ImGui UI code and application logic

Start by modifying the files in the `src/` directory to implement your own functionality.

### ImGui Configuration

Edit `imgui.ini` to customize the default window layout and settings. This file is automatically updated when you move or resize ImGui windows.

## Features

- ✅ Cross-platform support (Windows, Linux, macOS)
- ✅ Modern CMake setup with automatic dependency management
- ✅ ImGui integration with SDL3 backend
- ✅ Clean project structure for easy customization
- ✅ Debug and Release build configurations

## Troubleshooting

### Build Errors

If you encounter build errors, try cleaning and reconfiguring:

```powershell
# Remove the build directory
Remove-Item -Recurse -Force build

# Reconfigure
cmake -B build -G Ninja

# Build
cmake --build build
```

### Missing Dependencies

Ensure you have CMake and a C++ compiler installed. On Windows, you may need to run CMake from a Visual Studio Developer Command Prompt.

## License

This template is provided as-is for use in your projects. Please check the licenses of the included dependencies:
- [Dear ImGui License](https://github.com/ocornut/imgui/blob/master/LICENSE.txt)
- [SDL3 License](https://github.com/libsdl-org/SDL/blob/main/LICENSE.txt)

## Resources

- [Dear ImGui Documentation](https://github.com/ocornut/imgui)
- [ImGui Wiki](https://github.com/ocornut/imgui/wiki)
- [SDL3 Documentation](https://wiki.libsdl.org/SDL3/)

## Contributing

Feel free to fork this template and customize it for your needs. Consider sharing improvements back to the community!

---

**Happy coding!** 🚀
