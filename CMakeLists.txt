cmake_minimum_required(VERSION 3.8)
project(insta360gui)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rviz_common REQUIRED)
find_package(pluginlib REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

# ---- RViz2 panel plugin ----
add_library(${PROJECT_NAME}_rviz_plugins SHARED
  src/camera_perspective_panel.cpp
  include/insta360gui/camera_perspective_panel.hpp  # listed so AUTOMOC processes Q_OBJECT
)
target_include_directories(${PROJECT_NAME}_rviz_plugins PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(${PROJECT_NAME}_rviz_plugins Qt5::Widgets)
ament_target_dependencies(${PROJECT_NAME}_rviz_plugins
  rclcpp rviz_common pluginlib geometry_msgs tf2
)

pluginlib_export_plugin_description_file(rviz_common plugin_description.xml)

install(TARGETS ${PROJECT_NAME}_rviz_plugins
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

# ---- Standalone Tk slider (`ros2 run insta360gui camera_perspective_slider`) ----
install(PROGRAMS insta360gui/camera_perspective_slider.py
  DESTINATION lib/${PROJECT_NAME}
  RENAME camera_perspective_slider
)

ament_package()
