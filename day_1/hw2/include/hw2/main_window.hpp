#ifndef hw2_MAIN_WINDOW_H
#define hw2_MAIN_WINDOW_H

#include <QMainWindow>
#include <QIcon>
#include <QLabel>
#include <QSlider>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <opencv2/opencv.hpp>

#include "qnode.hpp"
#include "ui_mainwindow.h"

struct HSVRange
{
  int hueLow = 0, hueHigh = 179;
  int satLow = 0, satHigh = 255;
  int valLow = 0, valHigh = 255;
};

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();
  QNode* qnode;

private Q_SLOTS:
  void onNewImage(const cv::Mat& frame);
  void onRadioToggled();
  void onSliderChanged();

private:
  Ui::MainWindowDesign* ui;
  void closeEvent(QCloseEvent* event) override;

  void setupUiExtra();
  void loadRangeToSliders(const HSVRange& r);
  void showMatOnLabel(const cv::Mat& mat, QLabel* label);
  float findLineCenter(const cv::Mat& mask, bool vertical);
  bool findConeCentroidAndBox(const cv::Mat& mask, cv::Point2f& centroid, cv::Rect& box, double minArea = 200.0);
  QString classifyPosition(bool found, const cv::Point2f& center, float whiteX, float blueY, float band);

  // 화면 6분할
  QLabel *labelUsbCam_, *labelObject_, *labelNeon_, *labelWhite_, *labelBlue_, *labelOrange_;
  QLabel *labelNeonPos_, *labelOrangePos_;

  // 색상 선택 라디오버튼
  QRadioButton *radioWhite_, *radioBlue_, *radioNeon_, *radioOrange_;

  // HSV 슬라이더
  QSlider *sliderHueHigh_, *sliderHueLow_, *sliderSatHigh_, *sliderSatLow_, *sliderValHigh_, *sliderValLow_;
  QLabel *labelHueHigh_, *labelHueLow_, *labelSatHigh_, *labelSatLow_, *labelValHigh_, *labelValLow_;

  HSVRange rangeWhite_, rangeBlue_, rangeNeon_, rangeOrange_;
  HSVRange* currentRange_ = nullptr;
};

#endif  // hw2_MAIN_WINDOW_H