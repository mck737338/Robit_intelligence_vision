#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

namespace hw1
{

/**
 * @brief HSV 색상 범위 구조체
 */
struct HsvRange
{
  cv::Scalar lower;  ///< HSV 하한값
  cv::Scalar upper;  ///< HSV 상한값
  std::string name;  ///< 색상 이름 (저장 파일명에 사용)
};

/**
 * @brief HSV 기반 색상 검출 및 바이너리 이미지 변환 클래스
 */
class ColorDetector
{
public:
  ColorDetector();
  ~ColorDetector() = default;

  /**
   * @brief BGR 이미지를 HSV로 변환하고, 각 색상별 바이너리 마스크를 생성하여 저장
   * @param bgr_image  입력 BGR 이미지
   * @param save_dir   결과 이미지를 저장할 디렉터리 경로
   * @param prefix     저장 파일명 앞에 붙는 접두사 (예: "raw" or "gaussian")
   */
  void detect_and_save(const cv::Mat & bgr_image,
                       const std::string & save_dir,
                       const std::string & prefix);

  /**
   * @brief 이미지에 가우시안 필터를 적용한 후 detect_and_save 호출
   * @param bgr_image  입력 BGR 이미지
   * @param save_dir   결과 이미지를 저장할 디렉터리 경로
   * @param kernel_size 가우시안 커널 크기 (홀수, 기본값 5)
   * @param sigma       가우시안 표준편차 (기본값 0 → OpenCV가 자동 계산)
   */
  void apply_gaussian_and_detect(const cv::Mat & bgr_image,
                                 const std::string & save_dir,
                                 int kernel_size = 5,
                                 double sigma = 0);

private:
  /// 검출 대상 색상 목록 (빨강, 초록, 파랑)
  std::vector<HsvRange> color_ranges_;

  /**
   * @brief 빨간색의 경우 HSV 색조(Hue)가 0°와 180° 부근에 걸쳐 있으므로
   *        두 범위를 OR 연산으로 합산하는 헬퍼 함수
   */
  cv::Mat create_red_mask(const cv::Mat & hsv_image);
};

}  // namespace hw1
