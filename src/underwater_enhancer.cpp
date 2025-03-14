/*
 * Copyright(2025)
 */

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/types_c.h>
#include <underwater_enhancer/underwater_enhancer.hpp>

namespace underwaterEnhancer {
UnderwaterEnhancer::UnderwaterEnhancer() : Node("underwater_enhancer"), mUdcp_(false, 25){
  mResPub_ = create_publisher<sensor_msgs::msg::Image>("out", 10);
  mInSub_ = create_subscription<sensor_msgs::msg::Image>
              ("in", 10, std::bind(&UnderwaterEnhancer::processImage, this, std::placeholders::_1));
  mSetupSrv_ =
    create_service<std_srvs::srv::Empty>("toggle",
                                         std::bind(&UnderwaterEnhancer::toggleSetup, this,
                                                   std::placeholders::_1, std::placeholders::_2));
  mSetup_ = false;
  mHueMin_ = 0;
  mSatMin_ = 0;
  mValMin_ = 0;
  mHueMax_ = 255;
  mSatMax_ = 255;
  mValMax_ = 255;
}

void UnderwaterEnhancer::setupParameters(){}

void UnderwaterEnhancer::processImage(sensor_msgs::msg::Image::SharedPtr img){
  try {
    mCvPtr_ = cv_bridge::toCvCopy(img, sensor_msgs::image_encodings::BGR8);
  } catch (cv_bridge::Exception & e) {
    RCLCPP_ERROR(get_logger(), "Error while decoding image: %s", e.what());
    return;
  }

  // Converting the image in black and white to have better detection of the arucos
  mCurrentFrame_ = mCvPtr_->image;
  mEnhanced_ = mUdcp_.enhance(mCurrentFrame_);
  cv::Mat tmp;
  mCvPtr_->image = mEnhanced_;
  cv::waitKey(10);
  mResPub_->publish(*mCvPtr_->toImageMsg());

  if (mSetup_) {
    trackbarCallback(0, this);
    setupParameters();
  }
}
void UnderwaterEnhancer::toggleSetup(const std_srvs::srv::Empty::Request::SharedPtr,
                                     std_srvs::srv::Empty::Response::SharedPtr){
  mSetup_ = !mSetup_;
  if (mSetup_) {
    cv::namedWindow("Segmentation Result");
    cv::createTrackbar("Hue min", "Segmentation Result",
                       &mHueMin_, 255, &UnderwaterEnhancer::trackbarCallback, this);
    cv::createTrackbar("Sat min", "Segmentation Result",
                       &mValMin_, 255, &UnderwaterEnhancer::trackbarCallback, this);
    cv::createTrackbar("Val min", "Segmentation Result",
                       &mSatMin_, 255, &UnderwaterEnhancer::trackbarCallback, this);
    cv::createTrackbar("Hue max", "Segmentation Result",
                       &mHueMax_, 255, &UnderwaterEnhancer::trackbarCallback, this);
    cv::createTrackbar("Sat max", "Segmentation Result",
                       &mValMax_, 255, &UnderwaterEnhancer::trackbarCallback, this);
    cv::createTrackbar("Val max", "Segmentation Result",
                       &mSatMax_, 255, &UnderwaterEnhancer::trackbarCallback, this);
  } else {
    cv::destroyAllWindows();
  }
}
}  // namespace underwaterEnhancer
