/**
 * @file hsv_binary_node.cpp
 * @brief HsvBinaryNode 클래스 구현 및 main()
 *
 * 실행 방법:
 *   ros2 run hw1 hsv_binary_node --ros-args \
 *     -p image_path:=$(pwd)/src/hw1/image/source.jpg \
 *     -p save_dir:=$(pwd)/src/hw1/image
 */

#include "hw1/hsv_binary_node.hpp"

namespace hw1
{

HsvBinaryNode::HsvBinaryNode()
: Node("hsv_binary_node")
{
  load_parameters();

  if (!load_image()) {
    return;
  }

  run();
}

void HsvBinaryNode::load_parameters()
{
  this->declare_parameter<std::string>("image_path", "");
  this->declare_parameter<std::string>("save_dir",   "");

  image_path_ = this->get_parameter("image_path").as_string();
  save_dir_   = this->get_parameter("save_dir").as_string();

  RCLCPP_INFO(this->get_logger(), "=== HSV Binary Node ===");
  RCLCPP_INFO(this->get_logger(), "Image path : %s", image_path_.c_str());
  RCLCPP_INFO(this->get_logger(), "Save dir   : %s", save_dir_.c_str());

  if (image_path_.empty() || save_dir_.empty()) {
    RCLCPP_ERROR(this->get_logger(),
      "image_path and save_dir must be provided via --ros-args.\n"
      "  Example:\n"
      "  ros2 run hw1 hsv_binary_node --ros-args \\\n"
      "    -p image_path:=$(pwd)/src/hw1/image/source.jpg \\\n"
      "    -p save_dir:=$(pwd)/src/hw1/image");
  }
}

bool HsvBinaryNode::load_image()
{
  if (image_path_.empty()) {
    return false;
  }

  bgr_image_ = cv::imread(image_path_);

  if (bgr_image_.empty()) {
    RCLCPP_ERROR(this->get_logger(),
                 "Cannot load image: '%s'\n"
                 "  → Check that the file exists and the path is absolute.",
                 image_path_.c_str());
    return false;
  }

  RCLCPP_INFO(this->get_logger(),
              "Loaded image: %dx%d (channels=%d)",
              bgr_image_.cols, bgr_image_.rows, bgr_image_.channels());
  return true;
}

void HsvBinaryNode::run()
{
  detector_.detect_and_save(bgr_image_, save_dir_, "raw");

  RCLCPP_INFO(this->get_logger(),
              "Done. Binary images saved to: %s", save_dir_.c_str());
}

}  // namespace hw1

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<hw1::HsvBinaryNode>();
  rclcpp::shutdown();
  return 0;
}