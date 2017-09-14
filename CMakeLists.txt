cmake_minimum_required(VERSION 2.8)
project(fotomat)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/binaries)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/libraries)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

#file(GLOB_RECURSE sourceFiles src/*.cpp)

add_definitions("-std=c++11 -g -O3 -march=native -Wall -Wextra -Werror")

if(APPLE)
	set(QT5_PATH "/usr/local/opt/qt5" CACHE PATH "Path to Qt5")
	set(QT5_MODULE_PATH ${QT5_PATH}/lib/cmake)
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_MODULE_PATH})

	include_directories("/usr/local/include")
	link_directories("/usr/local/lib")

	add_definitions("-stdlib=libc++")
endif(APPLE)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include )
include_directories(${PROJECT_INCLUDE_DIR})

set(${CMAKE_PROJECT_NAME}_HEADERS
      ${PROJECT_INCLUDE_DIR}/CameraInterface.hpp  
      ${PROJECT_INCLUDE_DIR}/MainWindow.hpp     
      ${PROJECT_INCLUDE_DIR}/QRCodeGenerator.hpp  
      ${PROJECT_INCLUDE_DIR}/WorkerThread.hpp
      ${PROJECT_INCLUDE_DIR}/ImageSaver.hpp       
      ${PROJECT_INCLUDE_DIR}/OverlayWidget.hpp  
      ${PROJECT_INCLUDE_DIR}/VideoWidget.hpp
      )
set(${CMAKE_PROJECT_NAME}_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CameraInterface.cpp  
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MainWindow.cpp     
  ${CMAKE_CURRENT_SOURCE_DIR}/src/QRCodeGenerator.cpp  
  ${CMAKE_CURRENT_SOURCE_DIR}/src/WorkerThread.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImageSaver.cpp       
  ${CMAKE_CURRENT_SOURCE_DIR}/src/OverlayWidget.cpp  
  ${CMAKE_CURRENT_SOURCE_DIR}/src/VideoWidget.cpp      
)
set(sourceFiles 
      ${${CMAKE_PROJECT_NAME}_SOURCES}
      ${${CMAKE_PROJECT_NAME}_HEADERS}
      )

add_executable(${CMAKE_PROJECT_NAME} ${sourceFiles})

#Find the QRencode library
find_library(QRENCODE_LIBRARY
             NAMES qrencode
             HINTS /usr/lib /usr/lib/x86_64-linux-gnu
             REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} ${QRENCODE_LIBRARY})

#Find the GPhoto2 Library
find_library(GPHOTO2_LIBRARY
             NAMES gphoto2
             HINTS /usr/lib /usr/lib/x86_64-linux-gnu
             REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} ${GPHOTO2_LIBRARY})

#Find the GPhoto2 Port Library
find_library(GPHOTO2_PORT_LIBRARY
             NAMES gphoto2_port
             HINTS /usr/lib /usr/lib/x86_64-linux-gnu
             REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} ${GPHOTO2_PORT_LIBRARY})

# Find the Qt5Widgets library
find_package(Qt5Gui REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Gui)
include_directories(${Qt5Gui_INCLUDE_DIRS})

# Find the Qt5Widgets library
find_package(Qt5Widgets REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Widgets)
include_directories(${Qt5Widgets_INCLUDE_DIRS})

# Find the Qt5Core library
find_package(Qt5Core REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Core)
include_directories(${Qt5Core_INCLUDE_DIRS})

# Find the Qt5Network library
find_package(Qt5Network REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Network)
include_directories(${Qt5Network_INCLUDE_DIRS})

# Find the Qt5PrintSupport library
find_package(Qt5PrintSupport REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::PrintSupport)
include_directories(${Qt5PrintSupport_INCLUDE_DIRS})

# Find the Qt5Concurrent library
find_package(Qt5Concurrent REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Concurrent)
include_directories(${Qt5Concurrent_INCLUDE_DIRS})

find_package(Cups REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} ${CUPS_LIBRARIES})
include_directories(${CUPS_INCLUDE_DIRS})
