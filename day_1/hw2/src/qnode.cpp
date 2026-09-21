#include "../include/hw2/qnode.hpp"

QNode::QNode()
{
  int argc = 0;
  char** argv = nullptr;
  rclcpp::init(argc, argv);
  node = rclcpp::Node::make_shared("hw2");

  // usb_cam 등에서 퍼블리시하는 토픽 이름은 환경에 맞게 변경하세요.
  image_sub_ = node->create_subscription<sensor_msgs::msg::Image>(
      "/image_raw", rclcpp::SensorDataQoS(),
      std::bind(&QNode::imageCallback, this, std::placeholders::_1));

  this->start();
}

QNode::~QNode()
{
  if (rclcpp::ok())
  {
    rclcpp::shutdown();
  }
}

void QNode::imageCallback(const sensor_msgs::msg::Image::SharedPtr msg)
{
  RCLCPP_INFO_THROTTLE(
      node->get_logger(),
      *node->get_clock(),
      2000,
      "Image received: %d x %d, encoding: %s",
      msg->width,
      msg->height,
      msg->encoding.c_str());

  try
  {
    cv_bridge::CvImageConstPtr cv_ptr = cv_bridge::toCvShare(msg, "bgr8");
    cv::Mat frame = cv_ptr->image.clone();

    // Gaussian filter 적용
    cv::Mat blurred;
    cv::GaussianBlur(frame, blurred, cv::Size(5, 5), 0);

    Q_EMIT newImage(blurred);
  }
  catch (const cv_bridge::Exception& e)
  {
    RCLCPP_ERROR(node->get_logger(), "cv_bridge exception: %s", e.what());
  }
}

void QNode::run()
{
  rclcpp::WallRate loop_rate(20);
  while (rclcpp::ok())
  {
    rclcpp::spin_some(node);
    loop_rate.sleep();
  }
  rclcpp::shutdown();
  Q_EMIT rosShutDown();
}