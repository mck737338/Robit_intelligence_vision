/**
 * @file gaussian_binary_node.cpp
 * @brief GaussianBinaryNode 클래스 구현 및 main()
 *
 * 실행 방법:
 *   ros2 run hw1 gaussian_binary_node --ros-args \
 *     -p image_path:=$(pwd)/src/hw1/image/source.jpg \
 *     -p save_dir:=$(pwd)/src/hw1/image \
 *     -p kernel_size:=5 \
 *     -p sigma:=0.0
 */

#include "hw1/gaussian_binary_node.hpp"

namespace hw1
{

GaussianBinaryNode::GaussianBinaryNode()
: Node("gaussian_binary_node"),
  kernel_size_(5),
  sigma_(0.0)
{
  load_parameters();

  if (!load_image()) {
    return;
  }

  run();
}

void GaussianBinaryNode::load_parameters()
{
  this->declare_parameter<std::string>("image_path",  "");
  this->declare_parameter<std::string>("save_dir",    "");
  this->declare_parameter<int>        ("kernel_size", 5);
  this->declare_parameter<double>     ("sigma",       0.0);

  image_path_  = this->get_parameter("image_path").as_string();
  save_dir_    = this->get_parameter("save_dir").as_string();
  kernel_size_ = this->get_parameter("kernel_size").as_int();
  sigma_       = this->get_parameter("sigma").as_double();

  RCLCPP_INFO(this->get_logger(), "=== Gaussian Binary Node ===");
  RCLCPP_INFO(this->get_logger(), "Image path   : %s",   image_path_.c_str());
  RCLCPP_INFO(this->get_logger(), "Save dir     : %s",   save_dir_.c_str());
  RCLCPP_INFO(this->get_logger(), "Kernel size  : %d",   kernel_size_);
  RCLCPP_INFO(this->get_logger(), "Sigma        : %.2f", sigma_);

  if (image_path_.empty() || save_dir_.empty()) {
    RCLCPP_ERROR(this->get_logger(),
      "image_path and save_dir must be provided via --ros-args.\n"
      "  Example:\n"
      "  ros2 run hw1 gaussian_binary_node --ros-args \\\n"
      "    -p image_path:=$(pwd)/src/hw1/image/source.jpg \\\n"
      "    -p save_dir:=$(pwd)/src/hw1/image \\\n"
      "    -p kernel_size:=5 \\\n"
      "    -p sigma:=0.0");
  }
}

bool GaussianBinaryNode::load_image()
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

void GaussianBinaryNode::run()
{
  detector_.apply_gaussian_and_detect(bgr_image_, save_dir_, kernel_size_, sigma_);

  RCLCPP_INFO(this->get_logger(),
              "Done. Gaussian binary images saved to: %s", save_dir_.c_str());
}

}  // namespace hw1

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<hw1::GaussianBinaryNode>();
  rclcpp::shutdown();
  return 0;
}