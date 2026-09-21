#pragma once

/**
 * @file hsv_binary_node.hpp
 * @brief 원본 이미지를 HSV로 변환하여 빨강/초록/파랑 바이너리 이미지를 저장하는
 *        ROS2 노드 클래스 선언
 */

#include <rclcpp/rclcpp.hpp>
#include <opencv2/opencv.hpp>
#include "hw1/color_detector.hpp"

#include <string>

namespace hw1
{

/**
 * @class HsvBinaryNode
 * @brief 입력 이미지를 HSV 색상 공간으로 변환하고,
 *        cv::inRange() 를 이용하여 빨강·초록·파랑 공을 바이너리 이미지로 저장한다.
 *
 * ROS2 파라미터
 *   - image_path (string) : 입력 이미지 경로  (default: "image/source.jpg")
 *   - save_dir   (string) : 결과 저장 디렉터리 (default: "image")
 */
class HsvBinaryNode : public rclcpp::Node
{
public:
  /**
   * @brief 생성자 — 파라미터 로드, 이미지 로드, HSV 변환 및 저장을 순서대로 수행한다.
   */
  explicit HsvBinaryNode();

private:
  /**
   * @brief 파라미터를 읽어 멤버 변수에 저장한다.
   */
  void load_parameters();

  /**
   * @brief image_path_ 에서 이미지를 로드하여 bgr_image_ 에 저장한다.
   * @return 로드 성공 여부
   */
  bool load_image();

  /**
   * @brief HSV 변환 → inRange → 바이너리 이미지 저장을 수행한다.
   */
  void run();

  // ── 파라미터 ───────────────────────────────────────────────
  std::string image_path_;  ///< 입력 이미지 절대(또는 상대) 경로
  std::string save_dir_;    ///< 결과 이미지 저장 디렉터리

  // ── 데이터 ─────────────────────────────────────────────────
  cv::Mat        bgr_image_;  ///< 로드된 원본 BGR 이미지
  ColorDetector  detector_;   ///< HSV 색상 검출기
};

}  // namespace hw1
