cmake_minimum_required(VERSION 3.16)

# Sets project name
project(projects_2d LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sets C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specifies required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Gui)

# Specifies required Qt components
add_definitions(-D_USE_MATH_DEFINES)
add_definitions(-DTIXML_USE_STL)

# Specifies .cpp and .h files to be passed to the compiler
add_executable(${PROJECT_NAME}
  main.cpp

  mainwindow.cpp
  settings.cpp
  canvas2d.cpp

  mainwindow.h
  settings.h
  canvas2d.h
  rgba.h
)

# Specifies libraries to be linked (Qt components, glew, etc)
target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt::Core
  Qt::Widgets
  Qt::Gui
)

# Set this flag to silence warnings on Windows
if (MSVC OR MSYS OR MINGW)
  set(CMAKE_CXX_FLAGS "-Wno-volatile")
endif()

# Set this flag to silence warnings on MacOS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-volatile")
endif()
