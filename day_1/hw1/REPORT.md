# HW1 보고서: HSV 색상 검출 및 가우시안 필터 효과 분석

## 1. 개요

본 과제에서는 빨간공, 초록공, 파란공이 촬영된 이미지에서 OpenCV의 HSV 색상 공간과 `cv::inRange()`를 이용해 각 공을 바이너리 이미지로 분리한다. 이후 가우시안 필터를 전처리 단계로 추가하여 결과가 어떻게 달라지는지 비교·분석한다.

---

## 2. HSV 색상 공간과 inRange

### 2.1 왜 BGR 대신 HSV를 사용하는가?

| 속성 | BGR | HSV |
|------|-----|-----|
| 색상 표현 | 세 채널이 색상·밝기를 혼합 | H(색조)·S(채도)·V(명도)로 분리 |
| 조명 변화 민감도 | 높음 | 낮음 (V만 변화) |
| 색상 범위 지정 | 어려움 | 직관적·단순 |

HSV에서는 **색조(H)** 만으로 색상을 구분할 수 있으므로, 조명 강도 변화(V)나 채도(S) 변동에 강건하다.

### 2.2 OpenCV HSV 범위

OpenCV는 H를 0~180, S·V를 0~255로 스케일링한다.

| 색상 | H 범위 (OpenCV) | 비고 |
|------|-----------------|------|
| 빨강 | 0~10 **OR** 170~180 | 색조 환형 구조상 두 구간 필요 |
| 초록 | 35~85 | 단일 구간 |
| 파랑 | 100~130 | 단일 구간 |

### 2.3 inRange 동작 원리

```
cv::inRange(hsv, lower, upper, mask);
```

픽셀 (H, S, V) 각 채널이 [lower, upper] 범위에 모두 포함되면 `mask = 255`, 아니면 `mask = 0`으로 설정되어 **바이너리 이미지**가 생성된다.

---

## 3. 가우시안 필터(Gaussian Filter)란?

가우시안 필터는 커널(kernel) 내 픽셀을 가우시안 함수의 가중치로 평균내는 **저역통과 필터(Low-pass filter)** 이다.

```
G(x, y) = (1 / 2πσ²) × exp(-(x²+y²) / 2σ²)
```

- **커널 크기(k×k)**: 클수록 강한 블러 효과 (본 과제: 5×5)
- **σ(표준편차)**: 클수록 주변 픽셀 영향 증가

---

## 4. 원본 vs 가우시안 필터 후 비교

### 4.1 노이즈 제거 효과

| 항목 | 원본 이미지 | 가우시안 적용 후 |
|------|------------|-----------------|
| 노이즈 픽셀 | 고주파 노이즈 존재 → 마스크에 점 노이즈 발생 | 평활화로 노이즈 픽셀 제거 |
| 마스크 연속성 | 공 내부에 검은 구멍(hole) 발생 가능 | 균일한 흰색 영역으로 채워짐 |
| 경계(Edge) | 날카로운 경계 → 경계 픽셀 분류 불안정 | 경계 부근이 완만해져 안정적 분류 |

### 4.2 색상 경계 완화 효과

카메라 이미지에는 JPEG 압축 아티팩트, 조명 반사, 렌즈 왜곡 등으로 공의 가장자리 픽셀이 HSV 범위에서 벗어날 수 있다. 가우시안 블러는 경계 픽셀의 색상값을 주변과 혼합시켜 **경계 픽셀의 HSV 값을 "공 중심"과 가깝게** 이동시킨다.

### 4.3 단점

- 가우시안 블러는 에지를 희석시키므로, **정밀한 경계 추출**(예: Canny Edge)이 필요한 후속 작업에서는 오히려 불리할 수 있다.
- σ, 커널 크기가 너무 크면 색상이 인접 색상과 섞여 오검출이 발생한다.

---

## 5. 왜 가우시안 필터를 사용하는가? (결론)

1. **노이즈 억제**: 카메라 센서 노이즈·JPEG 압축 아티팩트를 제거하여 inRange 마스크에서 점 노이즈가 줄어든다.
2. **마스크 품질 향상**: 공 내부가 더 균일하게 255로 채워지므로 이후 모폴로지 연산(Morphology) 없이도 깔끔한 마스크를 얻을 수 있다.
3. **로봇 비전의 표준 전처리**: 조명이 균일하지 않은 실내 환경에서는 가우시안 블러가 사실상 표준 전처리 단계로 사용된다.
4. **연산 비용 대비 효과**: Gaussian은 분리가능(separable) 커널로 O(k)의 효율적인 구현이 가능하며, ROS2 실시간 처리에도 부담이 적다.

---

## 6. 파일 구조

```
hw1/
├── CMakeLists.txt
├── package.xml
├── REPORT.md
├── include/
│   └── hw1/
│       └── color_detector.hpp      # HSV 검출 클래스 헤더
├── src/
│   ├── color_detector.cpp           # HSV 검출 클래스 구현
│   ├── hsv_binary_node.cpp          # 원본 → HSV 바이너리 저장 노드
│   └── gaussian_binary_node.cpp     # 가우시안 후 HSV 바이너리 저장 노드
└── image/
    ├── source.jpg                   # 원본 입력 이미지
    ├── raw_binary_red.jpg           # 원본: 빨간공 바이너리
    ├── raw_binary_green.jpg         # 원본: 초록공 바이너리
    ├── raw_binary_blue.jpg          # 원본: 파란공 바이너리
    ├── raw_binary_all.jpg           # 원본: 전체 통합 바이너리
    ├── gaussian_blurred.jpg         # 가우시안 적용 이미지
    ├── gaussian_binary_red.jpg      # 가우시안 후: 빨간공 바이너리
    ├── gaussian_binary_green.jpg    # 가우시안 후: 초록공 바이너리
    ├── gaussian_binary_blue.jpg     # 가우시안 후: 파란공 바이너리
    └── gaussian_binary_all.jpg      # 가우시안 후: 전체 통합 바이너리
```

---

## 7. 빌드 및 실행

```bash
# 워크스페이스 루트에서
colcon build --packages-select hw1
source install/setup.bash

# Node 1: 원본 HSV 바이너리
ros2 run hw1 hsv_binary_node --ros-args \
  -p image_path:=$(pwd)/src/hw1/image/source.jpg \
  -p save_dir:=$(pwd)/src/hw1/image

# Node 2: 가우시안 필터 후 HSV 바이너리
ros2 run hw1 gaussian_binary_node --ros-args \
  -p image_path:=$(pwd)/src/hw1/image/source.jpg \
  -p save_dir:=$(pwd)/src/hw1/image \
  -p kernel_size:=5 \
  -p sigma:=0.0
```
