#include "hw1/color_detector.hpp"

#include <rclcpp/rclcpp.hpp>
#include <filesystem>
#include <stdexcept>

namespace hw1
{

// ---------------------------------------------------------------------------
// 생성자: 검출 대상 HSV 범위 초기화
// ---------------------------------------------------------------------------
ColorDetector::ColorDetector()
{
  /*
   * OpenCV HSV 범위
   *   H : 0 ~ 180  (360° / 2)
   *   S : 0 ~ 255
   *   V : 0 ~ 255
   *
   * 빨강(Red)  : Hue ≈ 0°  또는 ≈ 360° → 0~10 와 170~180 두 구간 사용
   * 초록(Green): Hue ≈ 60°  → 35~85
   * 파랑(Blue) : Hue ≈ 240° → 100~130
   */

  // 초록: 단일 구간
  color_ranges_.push_back({
    cv::Scalar(35, 80, 80),
    cv::Scalar(85, 255, 255),
    "green"
  });

  // 파랑: 단일 구간
  color_ranges_.push_back({
    cv::Scalar(100, 80, 80),
    cv::Scalar(130, 255, 255),
    "blue"
  });

  // 빨강은 두 구간이 필요하므로 color_ranges_ 에 넣지 않고
  // create_red_mask() 에서 별도 처리한다.
}

// ---------------------------------------------------------------------------
// 빨간색 마스크 (이중 구간 OR)
// ---------------------------------------------------------------------------
cv::Mat ColorDetector::create_red_mask(const cv::Mat & hsv_image)
{
  cv::Mat mask1, mask2, mask;
  cv::inRange(hsv_image, cv::Scalar(0, 80, 80),   cv::Scalar(10, 255, 255),  mask1);
  cv::inRange(hsv_image, cv::Scalar(170, 80, 80),  cv::Scalar(180, 255, 255), mask2);
  cv::bitwise_or(mask1, mask2, mask);
  return mask;
}

// ---------------------------------------------------------------------------
// 핵심 처리: HSV 변환 → inRange → 저장
// ---------------------------------------------------------------------------
void ColorDetector::detect_and_save(const cv::Mat & bgr_image,
                                    const std::string & save_dir,
                                    const std::string & prefix)
{
  // 저장 디렉터리 생성 (없으면)
  std::filesystem::create_directories(save_dir);

  // BGR → HSV 변환
  cv::Mat hsv_image;
  cv::cvtColor(bgr_image, hsv_image, cv::COLOR_BGR2HSV);

  // ── 빨강 ──────────────────────────────────────────────────
  cv::Mat red_mask = create_red_mask(hsv_image);
  std::string red_path = save_dir + "/" + prefix + "_binary_red.jpg";
  if (!cv::imwrite(red_path, red_mask)) {
    RCLCPP_ERROR(rclcpp::get_logger("color_detector"),
                 "Failed to save: %s", red_path.c_str());
  } else {
    RCLCPP_INFO(rclcpp::get_logger("color_detector"),
                "Saved: %s", red_path.c_str());
  }

  // ── 초록, 파랑 ────────────────────────────────────────────
  for (const auto & cr : color_ranges_) {
    cv::Mat mask;
    cv::inRange(hsv_image, cr.lower, cr.upper, mask);

    std::string path = save_dir + "/" + prefix + "_binary_" + cr.name + ".jpg";
    if (!cv::imwrite(path, mask)) {
      RCLCPP_ERROR(rclcpp::get_logger("color_detector"),
                   "Failed to save: %s", path.c_str());
    } else {
      RCLCPP_INFO(rclcpp::get_logger("color_detector"),
                  "Saved: %s", path.c_str());
    }
  }

  // ── 세 마스크 합산 (전체 컬러 볼 통합 바이너리) ───────────
  cv::Mat green_mask, blue_mask, combined;
  cv::inRange(hsv_image, color_ranges_[0].lower, color_ranges_[0].upper, green_mask);
  cv::inRange(hsv_image, color_ranges_[1].lower, color_ranges_[1].upper, blue_mask);
  cv::bitwise_or(red_mask, green_mask, combined);
  cv::bitwise_or(combined, blue_mask, combined);

  std::string combined_path = save_dir + "/" + prefix + "_binary_all.jpg";
  if (!cv::imwrite(combined_path, combined)) {
    RCLCPP_ERROR(rclcpp::get_logger("color_detector"),
                 "Failed to save: %s", combined_path.c_str());
  } else {
    RCLCPP_INFO(rclcpp::get_logger("color_detector"),
                "Saved: %s", combined_path.c_str());
  }
}

// ---------------------------------------------------------------------------
// 가우시안 필터 적용 후 동일 처리
// ---------------------------------------------------------------------------
void ColorDetector::apply_gaussian_and_detect(const cv::Mat & bgr_image,
                                              const std::string & save_dir,
                                              int kernel_size,
                                              double sigma)
{
  // kernel_size 는 반드시 홀수이어야 한다
  if (kernel_size % 2 == 0) {
    RCLCPP_WARN(rclcpp::get_logger("color_detector"),
                "kernel_size must be odd. Incrementing %d -> %d",
                kernel_size, kernel_size + 1);
    kernel_size += 1;
  }

  cv::Mat blurred;
  cv::GaussianBlur(bgr_image, blurred, cv::Size(kernel_size, kernel_size), sigma);

  // 가우시안 필터 적용 이미지 자체도 저장
  std::filesystem::create_directories(save_dir);
  std::string blurred_path = save_dir + "/gaussian_blurred.jpg";
  cv::imwrite(blurred_path, blurred);
  RCLCPP_INFO(rclcpp::get_logger("color_detector"),
              "Saved gaussian blurred image: %s", blurred_path.c_str());

  detect_and_save(blurred, save_dir, "gaussian");
}

}  // namespace hw1
