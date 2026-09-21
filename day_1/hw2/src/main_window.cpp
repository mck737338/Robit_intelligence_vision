#include <QDebug>
#include "../include/hw2/main_window.hpp"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindowDesign)
{
  ui->setupUi(this);

  QIcon icon("://ros-icon.png");
  this->setWindowIcon(icon);

  qRegisterMetaType<cv::Mat>("cv::Mat");

  setupUiExtra();

  qnode = new QNode();
  QObject::connect(qnode, SIGNAL(rosShutDown()), this, SLOT(close()));
  QObject::connect(qnode, &QNode::newImage, this, &MainWindow::onNewImage, Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  QMainWindow::closeEvent(event);
}

// ---------------------------------------------------------------------------
// UI 구성 (6분할 화면 + 색상 선택 라디오버튼 + HSV 슬라이더)
// ---------------------------------------------------------------------------
void MainWindow::setupUiExtra()
{
  QWidget* central = new QWidget(this);
  QVBoxLayout* mainLayout = new QVBoxLayout(central);

  QGridLayout* grid = new QGridLayout();
  auto makePanel = [&](QLabel*& img, const QString& title, int row, int col) {
    QLabel* titleLabel = new QLabel(title);
    img = new QLabel();
    img->setMinimumSize(280, 190);
    img->setFrameShape(QFrame::Box);
    img->setAlignment(Qt::AlignCenter);
    grid->addWidget(titleLabel, row, col);
    grid->addWidget(img, row + 1, col);
  };

  makePanel(labelUsbCam_, "Usb_Cam", 0, 0);
  makePanel(labelObject_, "Find_object", 0, 1);
  makePanel(labelNeon_,   "Neon_cone", 0, 2);
  makePanel(labelWhite_,  "White_line", 2, 0);
  makePanel(labelBlue_,   "Blue_line", 2, 1);
  makePanel(labelOrange_, "Orange_cone", 2, 2);

  mainLayout->addLayout(grid);

  QHBoxLayout* controls = new QHBoxLayout();

  // 라디오버튼: 어느 색상 슬라이더를 조정할지 선택
  QButtonGroup* group = new QButtonGroup(this);
  radioWhite_  = new QRadioButton("White_line");
  radioBlue_   = new QRadioButton("Blue_line");
  radioNeon_   = new QRadioButton("Neon_cone");
  radioOrange_ = new QRadioButton("Orange_cone");
  radioWhite_->setChecked(true);
  group->addButton(radioWhite_);
  group->addButton(radioBlue_);
  group->addButton(radioNeon_);
  group->addButton(radioOrange_);

  QVBoxLayout* radioLayout = new QVBoxLayout();
  radioLayout->addWidget(radioWhite_);
  radioLayout->addWidget(radioBlue_);
  radioLayout->addWidget(radioNeon_);
  radioLayout->addWidget(radioOrange_);
  controls->addLayout(radioLayout);

  auto makeSliderColumn = [&](const QString& title, QSlider*& high, QSlider*& low,
                               QLabel*& highLbl, QLabel*& lowLbl, int maxVal) {
    QVBoxLayout* col = new QVBoxLayout();
    col->addWidget(new QLabel(title));
    highLbl = new QLabel("High: 0");
    high = new QSlider(Qt::Horizontal);
    high->setRange(0, maxVal);
    lowLbl = new QLabel("Low: 0");
    low = new QSlider(Qt::Horizontal);
    low->setRange(0, maxVal);
    col->addWidget(highLbl);
    col->addWidget(high);
    col->addWidget(lowLbl);
    col->addWidget(low);
    controls->addLayout(col);
  };

  makeSliderColumn("Hue", sliderHueHigh_, sliderHueLow_, labelHueHigh_, labelHueLow_, 179);
  makeSliderColumn("Saturation", sliderSatHigh_, sliderSatLow_, labelSatHigh_, labelSatLow_, 255);
  makeSliderColumn("Value", sliderValHigh_, sliderValLow_, labelValHigh_, labelValLow_, 255);

  QVBoxLayout* posLayout = new QVBoxLayout();
  labelNeonPos_ = new QLabel("Neon: 존재하지 않음");
  labelOrangePos_ = new QLabel("Orange: 존재하지 않음");
  posLayout->addWidget(labelNeonPos_);
  posLayout->addWidget(labelOrangePos_);
  controls->addLayout(posLayout);

  mainLayout->addLayout(controls);
  this->setCentralWidget(central);

  currentRange_ = &rangeWhite_;
  loadRangeToSliders(*currentRange_);

  connect(radioWhite_,  &QRadioButton::toggled, this, &MainWindow::onRadioToggled);
  connect(radioBlue_,   &QRadioButton::toggled, this, &MainWindow::onRadioToggled);
  connect(radioNeon_,   &QRadioButton::toggled, this, &MainWindow::onRadioToggled);
  connect(radioOrange_, &QRadioButton::toggled, this, &MainWindow::onRadioToggled);

  connect(sliderHueHigh_, &QSlider::valueChanged, this, &MainWindow::onSliderChanged);
  connect(sliderHueLow_,  &QSlider::valueChanged, this, &MainWindow::onSliderChanged);
  connect(sliderSatHigh_, &QSlider::valueChanged, this, &MainWindow::onSliderChanged);
  connect(sliderSatLow_,  &QSlider::valueChanged, this, &MainWindow::onSliderChanged);
  connect(sliderValHigh_, &QSlider::valueChanged, this, &MainWindow::onSliderChanged);
  connect(sliderValLow_,  &QSlider::valueChanged, this, &MainWindow::onSliderChanged);
}

void MainWindow::onRadioToggled()
{
  QRadioButton* btn = qobject_cast<QRadioButton*>(sender());
  if (!btn || !btn->isChecked()) return;

  if (btn == radioWhite_)       currentRange_ = &rangeWhite_;
  else if (btn == radioBlue_)   currentRange_ = &rangeBlue_;
  else if (btn == radioNeon_)   currentRange_ = &rangeNeon_;
  else if (btn == radioOrange_) currentRange_ = &rangeOrange_;

  loadRangeToSliders(*currentRange_);
}

void MainWindow::loadRangeToSliders(const HSVRange& r)
{
  auto setSlider = [](QSlider* s, int v) {
    s->blockSignals(true);
    s->setValue(v);
    s->blockSignals(false);
  };
  setSlider(sliderHueHigh_, r.hueHigh);
  setSlider(sliderHueLow_,  r.hueLow);
  setSlider(sliderSatHigh_, r.satHigh);
  setSlider(sliderSatLow_,  r.satLow);
  setSlider(sliderValHigh_, r.valHigh);
  setSlider(sliderValLow_,  r.valLow);

  labelHueHigh_->setText("High: " + QString::number(r.hueHigh));
  labelHueLow_->setText("Low: " + QString::number(r.hueLow));
  labelSatHigh_->setText("High: " + QString::number(r.satHigh));
  labelSatLow_->setText("Low: " + QString::number(r.satLow));
  labelValHigh_->setText("High: " + QString::number(r.valHigh));
  labelValLow_->setText("Low: " + QString::number(r.valLow));
}

void MainWindow::onSliderChanged()
{
  if (!currentRange_) return;
  currentRange_->hueHigh = sliderHueHigh_->value();
  currentRange_->hueLow  = sliderHueLow_->value();
  currentRange_->satHigh = sliderSatHigh_->value();
  currentRange_->satLow  = sliderSatLow_->value();
  currentRange_->valHigh = sliderValHigh_->value();
  currentRange_->valLow  = sliderValLow_->value();

  labelHueHigh_->setText("High: " + QString::number(currentRange_->hueHigh));
  labelHueLow_->setText("Low: " + QString::number(currentRange_->hueLow));
  labelSatHigh_->setText("High: " + QString::number(currentRange_->satHigh));
  labelSatLow_->setText("Low: " + QString::number(currentRange_->satLow));
  labelValHigh_->setText("High: " + QString::number(currentRange_->valHigh));
  labelValLow_->setText("Low: " + QString::number(currentRange_->valLow));
}

// ---------------------------------------------------------------------------
// 영상 처리
// ---------------------------------------------------------------------------
void MainWindow::showMatOnLabel(const cv::Mat& mat, QLabel* label)
{
  QImage img;
  if (mat.channels() == 1)
  {
    img = QImage(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8).copy();
  }
  else
  {
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    img = QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888).copy();
  }
  label->setPixmap(QPixmap::fromImage(img).scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

float MainWindow::findLineCenter(const cv::Mat& mask, bool vertical)
{
  cv::Moments m = cv::moments(mask, true);
  if (m.m00 < 1e-3)
  {
    return vertical ? mask.cols / 2.0f : mask.rows / 2.0f;
  }
  return vertical ? static_cast<float>(m.m10 / m.m00) : static_cast<float>(m.m01 / m.m00);
}

bool MainWindow::findConeCentroidAndBox(const cv::Mat& mask, cv::Point2f& centroid, cv::Rect& box, double minArea)
{
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  double bestArea = 0.0;
  int bestIdx = -1;
  for (size_t i = 0; i < contours.size(); ++i)
  {
    double a = cv::contourArea(contours[i]);
    if (a > bestArea)
    {
      bestArea = a;
      bestIdx = static_cast<int>(i);
    }
  }
  if (bestIdx < 0 || bestArea < minArea) return false;

  box = cv::boundingRect(contours[bestIdx]);
  cv::Moments m = cv::moments(contours[bestIdx]);
  centroid = cv::Point2f(static_cast<float>(m.m10 / m.m00), static_cast<float>(m.m01 / m.m00));
  return true;
}

QString MainWindow::classifyPosition(bool found, const cv::Point2f& center, float whiteX, float blueY, float band)
{
  if (!found) return "존재하지 않음";

  bool onWhite = std::fabs(center.x - whiteX) < band;
  bool onBlue  = std::fabs(center.y - blueY) < band;

  if (onWhite && onBlue) return "중앙";
  if (onWhite) return (center.y < blueY) ? "위쪽 하얀선 위" : "아래쪽 하얀선 위";
  if (onBlue)  return (center.x < whiteX) ? "왼쪽 파란선 위" : "오른쪽 파란선 위";

  bool right = center.x > whiteX;
  bool top   = center.y < blueY;  // 이미지 좌표계는 y가 아래로 증가
  if (right && top)   return "1사분면";
  if (!right && top)  return "2사분면";
  if (!right && !top) return "3사분면";
  return "4사분면";
}

void MainWindow::onNewImage(const cv::Mat& frame)
{
  if (frame.empty()) return;

  qDebug() << "frame:"
         << frame.cols << "x" << frame.rows
         << "channels:" << frame.channels();

  cv::Mat hsv;
  cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

  auto makeMask = [&](const HSVRange& r) {
    cv::Mat m;
    cv::inRange(hsv, cv::Scalar(r.hueLow, r.satLow, r.valLow),
                cv::Scalar(r.hueHigh, r.satHigh, r.valHigh), m);
    cv::morphologyEx(m, m, cv::MORPH_OPEN,
                      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));
    return m;
  };

  cv::Mat whiteMask  = makeMask(rangeWhite_);
  cv::Mat blueMask   = makeMask(rangeBlue_);
  cv::Mat neonMask   = makeMask(rangeNeon_);
  cv::Mat orangeMask = makeMask(rangeOrange_);

  qDebug() << "white:"
         << cv::countNonZero(whiteMask)
         << "blue:"
         << cv::countNonZero(blueMask)
         << "neon:"
         << cv::countNonZero(neonMask)
         << "orange:"
         << cv::countNonZero(orangeMask);

  float whiteX = findLineCenter(whiteMask, true);
  float blueY  = findLineCenter(blueMask, false);

  cv::Point2f neonC, orangeC;
  cv::Rect neonBox, orangeBox;
  bool neonFound   = findConeCentroidAndBox(neonMask, neonC, neonBox);
  bool orangeFound = findConeCentroidAndBox(orangeMask, orangeC, orangeBox);

  // 콘이 인식된 경우 바운딩박스 내부만 추출 (필요 시 이후 처리/저장에 활용)
  cv::Mat neonCrop, orangeCrop;
  if (neonFound)   neonCrop   = frame(neonBox).clone();
  if (orangeFound) orangeCrop = frame(orangeBox).clone();

  cv::Mat overlay = frame.clone();
  cv::line(overlay, cv::Point(static_cast<int>(whiteX), 0),
           cv::Point(static_cast<int>(whiteX), overlay.rows), cv::Scalar(0, 255, 255), 2);
  cv::line(overlay, cv::Point(0, static_cast<int>(blueY)),
           cv::Point(overlay.cols, static_cast<int>(blueY)), cv::Scalar(255, 0, 255), 2);
  if (neonFound)   cv::rectangle(overlay, neonBox, cv::Scalar(255, 0, 0), 2);
  if (orangeFound) cv::rectangle(overlay, orangeBox, cv::Scalar(0, 0, 255), 2);

  const float band = 15.0f;
  QString neonPos   = classifyPosition(neonFound, neonC, whiteX, blueY, band);
  QString orangePos = classifyPosition(orangeFound, orangeC, whiteX, blueY, band);

  labelNeonPos_->setText("Neon: " + neonPos);
  labelOrangePos_->setText("Orange: " + orangePos);

  showMatOnLabel(frame, labelUsbCam_);
  showMatOnLabel(overlay, labelObject_);
  showMatOnLabel(neonMask, labelNeon_);
  showMatOnLabel(whiteMask, labelWhite_);
  showMatOnLabel(blueMask, labelBlue_);
  showMatOnLabel(orangeMask, labelOrange_);
}