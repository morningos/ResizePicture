#include "mainwindow.h"

#include <QActionGroup>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QMessageBox>
#include <QPixmap>
#include <QStandardPaths>
#include <opencv2/opencv.hpp>

#include "aboutdialog.h"
#include "cropwindow.h"
#include "error.h"
#include "file.h"
#include "imageinfodialog.h"
#include "imageutils.h"
#include "ui_imageinfodialog.h"
#include "ui_mainwindow.h"

using namespace cv;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  this->setWindowIcon(
      QIcon(QApplication::applicationDirPath() + "/etc/images/ic_app.png"));

  connect(ui->actionOpenFile, SIGNAL(triggered()), this, SLOT(openImage()));
  connect(ui->actionZoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()));
  connect(ui->actionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));
  connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveImage()));
  connect(ui->actionResetImage, SIGNAL(triggered(bool)), this,
          SLOT(reset(bool)));
  connect(ui->actionCanny, SIGNAL(triggered(bool)), this,
          SLOT(cannyCheck(bool)));
  connect(ui->actionGrayImage, SIGNAL(triggered(bool)), this,
          SLOT(grayImage(bool)));
  connect(ui->actionDenoising, SIGNAL(triggered()), this, SLOT(denoising()));
  connect(ui->brightnessSlider, SIGNAL(valueChanged(int)), this,
          SLOT(changBrightness(int)));
  ui->screen->setCacheMode(QGraphicsView::CacheNone);
  ui->screen->setDragMode(QGraphicsView::ScrollHandDrag);

  scene = new QGraphicsScene;

  imageOperationMenu = new QActionGroup(this);
  imageOperationMenu->addAction(ui->actionResetImage);
  imageOperationMenu->addAction(ui->actionCanny);
  imageOperationMenu->addAction(ui->actionGrayImage);
}

MainWindow::~MainWindow() {
  if (!tempImagePath.isEmpty() && QFile::exists(tempImagePath)) {
    QFile::remove(tempImagePath);
  }
  if (imageUtils) {
    delete imageUtils;
  }
  delete imageOperationMenu;
  delete scene;
  delete ui;
}

void MainWindow::openImage() {
  sourceImagePath = QFileDialog::getOpenFileName(
      this, "选择一张图片",
      QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
      "图片(*.png;*.jpeg;*.jpg;*.bmp;*.webp;*.tiff);;所有文件(*.*)");
  if (sourceImagePath.isEmpty()) {
    return;
  }
  setWindowTitle(sourceImagePath);
  tempImagePath = File::generateTempSourceFile(sourceImagePath);
  imageUtils = new ImageUtils(tempImagePath);
  src = imread(this->tempImagePath.toStdString());
  if (!QFile::exists(tempImagePath)) {
    QMessageBox::critical(this, "错误！", "图片读取失败");
    return;
  }
  this->times = 1.00;
  ui->actionResetImage->setChecked(true);
  ui->actionSave->setEnabled(true);
  ui->actionGetImageInfo->setEnabled(true);
  ui->actionZoomIn->setEnabled(true);
  ui->actionZoomOut->setEnabled(true);
  ui->actionResetImage->setEnabled(true);
  ui->actionCanny->setEnabled(true);
  ui->actionGrayImage->setEnabled(true);
  ui->actionDenoising->setEnabled(true);
  ui->actionCropImage->setEnabled(true);
  ui->brightnessSlider->setValue(50);
  showImage(tempImagePath);
}

void MainWindow::saveImage() { File::saveImage(this, tempImagePath); }

void MainWindow::reset(bool isChecked) {
  if (isChecked) {
    tempImagePath = File::generateTempSourceFile(sourceImagePath);
    imageUtils = new ImageUtils(tempImagePath);
    times = 1.0;
    showImage(tempImagePath);
  }
}

void MainWindow::cannyCheck(bool isChecked) {
  if (isChecked) {
    tempImagePath = imageUtils->cannyCheck(tempImagePath);
    imageUtils = new ImageUtils(tempImagePath);
    times = 1.0;
    showImage(tempImagePath);
  }
}

void MainWindow::grayImage(bool isChecked) {
  if (isChecked) {
    tempImagePath = imageUtils->generateGrayImage(tempImagePath);
    imageUtils = new ImageUtils(tempImagePath);
    times = 1.0;
    showImage(tempImagePath);
  }
}

void MainWindow::denoising() {
  QString deNoiseImagePath = imageUtils->denoisingImage();
  if (deNoiseImagePath.startsWith("-")) {
    return;
  }
  tempImagePath = deNoiseImagePath;
  imageUtils = new ImageUtils(tempImagePath);
  times = 1.0;
  showImage(tempImagePath);
}

void MainWindow::changBrightness(int level) {
  if (src.empty()) {
    return;
  }
  int height = src.rows;
  int width = src.cols;
  int channels = src.channels();

  float alpha = level / 50.0;
  float beta = 0;

  //初始化dst为与src同大小、类型的空白图片
  Mat dst = Mat::zeros(src.size(), src.type());
  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      //单通道
      if (channels == 1) {
        float v = src.at<uchar>(row, col);
        dst.at<uchar>(row, col) = saturate_cast<uchar>(v * alpha + beta);
      }
      //三通道
      else if (channels == 3) {
        float b = src.at<Vec3b>(row, col)[0];
        float g = src.at<Vec3b>(row, col)[1];
        float r = src.at<Vec3b>(row, col)[2];
        dst.at<Vec3b>(row, col)[0] = saturate_cast<uchar>(b * alpha + beta);
        dst.at<Vec3b>(row, col)[1] = saturate_cast<uchar>(g * alpha + beta);
        dst.at<Vec3b>(row, col)[2] = saturate_cast<uchar>(r * alpha + beta);
      }
    }
  }
  showImage(ImageUtils::mat2QImage(dst));
}

void MainWindow::zoomIn() {
  times += 0.2;
  zoom();
}

void MainWindow::zoomOut() {
  times -= 0.2;
  zoom();
}

void MainWindow::zoom() {
  if (tempImagePath.isEmpty()) {
    ui->statusbar->showMessage("操作错误，图片不存在");
    return;
  }
  if (times <= imageUtils->MIN_TIMES || times > imageUtils->MAX_TIMES ||
      !times) {
    QMessageBox::information(this, "提示", "已经达到缩放极限");
    return;
  }
  QString tempPath = imageUtils->zoom(times);
  if (tempPath.startsWith("-")) {
    QMessageBox::information(this, "错误", "操作错误，错误码：" + tempPath);
    return;
  }
  this->showImage(tempPath);
}

void MainWindow::showImage(const QString& imagePath) {
  if (imagePath.isEmpty() || !QFile::exists(imagePath)) {
    return;
  }
  if (scene) {
    delete scene;
    scene = new QGraphicsScene;
  }
  QPixmap* pixmap = new QPixmap(imagePath);
  scene->addPixmap(*pixmap);
  ui->screen->setScene(scene);
  ui->screen->show();
  updateStatusBar();
}

void MainWindow::showImage(const QImage& image) {
  if (scene) {
    delete scene;
    scene = new QGraphicsScene;
  }
  scene->addPixmap(QPixmap::fromImage(image));
  ui->screen->setScene(scene);
  ui->screen->show();
  updateStatusBar();
}

void MainWindow::updateStatusBar() const {
  ui->statusbar->showMessage("缩放倍数:" + QString::number(times));
}

void MainWindow::on_actionCropImage_triggered() {
  CropWindow* cropWindow = new CropWindow;
  cropWindow->setSourcePath(tempImagePath);
  cropWindow->show();
}

void MainWindow::on_actionShowAbout_triggered() {
  AboutDialog* dialog = new AboutDialog;
  dialog->exec();
}

void MainWindow::on_actionGetImageInfo_triggered() {
  if (sourceImagePath.isEmpty()) {
    return;
  }
  ImageInfoDialog* dialog = new ImageInfoDialog(sourceImagePath);
  dialog->exec();
}
