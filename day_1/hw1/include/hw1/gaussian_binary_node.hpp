#pragma once

/**
 * @file gaussian_binary_node.hpp
 * @brief 가우시안 필터를 먼저 적용한 뒤 HSV 바이너리 이미지를 저장하는
 *        ROS2 노드 클래스 선언
 */

#include <rclcpp/rclcpp.hpp>
#include <opencv2/opencv.hpp>
#include "hw1/color_detector.hpp"

#include <string>

namespace hw1
{

/**
 * @class GaussianBinaryNode
 * @brief 입력 이미지에 가우시안 블러를 먼저 적용한 뒤,
 *        HSV 색상 공간에서 cv::inRange() 로 빨강·초록·파랑 공을
 *        바이너리 이미지로 저장한다.
 *
 * ROS2 파라미터
 *   - image_path  (string) : 입력 이미지 경로        (default: "image/source.jpg")
 *   - save_dir    (string) : 결과 저장 디렉터리       (default: "image")
 *   - kernel_size (int)    : 가우시안 커널 크기 (홀수) (default: 5)
 *   - sigma       (double) : 가우시안 표준편차         (default: 0.0 → OpenCV 자동)
 */
class GaussianBinaryNode : public rclcpp::Node
{
public:
  /**
   * @brief 생성자 — 파라미터 로드, 이미지 로드, 가우시안 블러 후 HSV 변환 및 저장을 수행한다.
   */
  explicit GaussianBinaryNode();

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
   * @brief 가우시안 필터 적용 → HSV 변환 → inRange → 바이너리 이미지 저장을 수행한다.
   */
  void run();

  // ── 파라미터 ───────────────────────────────────────────────
  std::string image_path_;   ///< 입력 이미지 절대(또는 상대) 경로
  std::string save_dir_;     ///< 결과 이미지 저장 디렉터리
  int         kernel_size_;  ///< 가우시안 커널 크기 (반드시 홀수)
  double      sigma_;        ///< 가우시안 표준편차 (0이면 OpenCV가 커널 크기로부터 자동 산출)

  // ── 데이터 ─────────────────────────────────────────────────
  cv::Mat        bgr_image_;  ///< 로드된 원본 BGR 이미지
  ColorDetector  detector_;   ///< HSV 색상 검출기
};

}  // namespace hw1
