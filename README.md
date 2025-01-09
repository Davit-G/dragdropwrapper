# dragdropwrapper  🫳    💨 🎁 

dragdropwrapper is a lightweight C++ library with one mission: arbitrarily trigger outbound file drag and drop events with one function call across different OSes.

# Notes
- This is a small static library that I want to use on my own projects with CMake, so expect a bit of jank.
- It is blocking.
- Since I use it for my own projects don't expect all systems to behave correctly.
- If you'd like to use it too feel free but this code could be prone to issues and bugs, I tried my best 🤷

I plan to add a bit of basic error handling.

## OS Support

dragdropwrapper should be able to support:
- Windows 2000 and above (incl. Windows 11, 10, 7, XP, etc) via [ole2.h](https://learn.microsoft.com/en-us/windows/win32/api/ole2/nf-ole2-dodragdrop)
- MacOS 10.0+ via [AppKit](https://developer.apple.com/documentation/appkit/drag-and-drop)

dragdropwrapper has been tested on:
- Windows 10 Pro 22H2, using MSVC (Visual Studio Build Tools 2022 Release - x86_amd64), C++
- MacOS Sequoia 15.2, using Clang 16.0.0 arm64 (Apple Darwin 24.2.0), with Objective C++

I want to add support for other Linux systems running X11 (or Wayland?) in the future, and as this gets done, this repo will get updated.

# Installation (Cmake)
To add this library to an existing CMake project, make sure you have FetchContent included in your CMakeLists.txt somewhere (preferably at the top of your file):
```cmake
include(FetchContent) # include this at the top of your file if you haven't already
set(FETCHCONTENT_QUIET FALSE) # Optional: display git progress information from FetchContent. Dont include it if you dont need it
```

Download *dragdropwrapper*:
```
FetchContent_Declare(
    dragdropwrapper
    GIT_REPOSITORY https://github.com/Davit-G/dragdropwrapper
    GIT_TAG "main" # you can change this to any commit hash, tag or branch
    GIT_PROGRESS TRUE # shows the download progress. You can remove this if it's not needed
    GIT_SHALLOW 1 # only clone the latest commit
)

FetchContent_MakeAvailable(dragdropwrapper)
```

Link it to your project:
```
target_link_libraries(${PROJECT_NAME} PRIVATE dragdropwrapper <... other linked libraries>)
```

# Usage
Import it into your project:

```c++
#include "dragdropwrapper.h"
```

You will now have access to a convenient method, `SendFileAsDragDrop`.
```c++
void SendFileAsDragDrop(void* handle, const char* file_path, std::function<void(void)> callback);
```
Arguments:
- `void* handle`:
    - Window handle (On Mac this is a NSWindow). On Windows the handle argument doesn't get used, so feel free to pass a nullptr.
- `const char* file_path`:
    - An absolute file path which is the target file you want to call a system file drag with.
- `std::function<void(void)> callback`:
    - Callback function that you can run after it's successfully finished dragging.

The suggested workflow to use this library is to:
- Capture a mouse drag event from your program / gui library / component
- Save the data you want to drag and drop as a file and store it's absolute path.
- Then, provide that path to `SendFileAsDragDrop` on a separate thread (to avoid blocking your code).



# Important Notes:
`SendFileAsDragDrop` is a blocking function.

On MacOS the window handle is mandatory:
  - The window handle should be your program's [NSWindow](https://developer.apple.com/documentation/appkit/nswindow) handle.
  - If you are creating your own NSWindow you can pass the window handle directly, but if you are using a graphics library such as JUCE, raylib, etc, you might be able to grab a window handle from it.
    - For example, Raylib has a convenient method [GetWindowHandle()](https://www.raylib.com/cheatsheet/cheatsheet.html) which does this exact operation.

SendFileAsDragDrop does not currently support arbitrary text / images or other assets yet, this could be in the works in the future.

## Contribution
This repository is the most boring thing I've written, if you'd like to contribute fixes or patches (please do 😅), then submit an issue or a pull request.

Thanks!
