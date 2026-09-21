cmake_minimum_required(VERSION 3.8)
project(hw1)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(include)

# === 공유 라이브러리: color_detector ===
add_library(color_detector src/color_detector.cpp)
ament_target_dependencies(color_detector
  rclcpp
  OpenCV
)
target_include_directories(color_detector PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# === Node 1: hsv_binary_node ===
# 소스: src/hsv_binary_node.cpp
# 헤더: include/hw1/hsv_binary_node.hpp
add_executable(hsv_binary_node src/hsv_binary_node.cpp)
target_link_libraries(hsv_binary_node color_detector)
ament_target_dependencies(hsv_binary_node
  rclcpp
  OpenCV
)
target_include_directories(hsv_binary_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# === Node 2: gaussian_binary_node ===
# 소스: src/gaussian_binary_node.cpp
# 헤더: include/hw1/gaussian_binary_node.hpp
add_executable(gaussian_binary_node src/gaussian_binary_node.cpp)
target_link_libraries(gaussian_binary_node color_detector)
ament_target_dependencies(gaussian_binary_node
  rclcpp
  OpenCV
)
target_include_directories(gaussian_binary_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(TARGETS
  color_detector
  hsv_binary_node
  gaussian_binary_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

# image 폴더를 설치 경로에 포함
install(DIRECTORY image/
  DESTINATION share/${PROJECT_NAME}/image
)

ament_package()
