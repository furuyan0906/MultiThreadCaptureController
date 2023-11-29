#include <iostream>
#include <string>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <gtest/gtest.h>
#include "common/MultiThreadCaptureController.hpp"
#include "cv/CvCapture.hpp"


#ifndef NDEBUG
constexpr bool is_dbg_ = true;
#else
constexpr bool is_dbg_ = false;
#endif
constexpr bool is_cap_delete_ = true;

static constexpr int dev_ = 0;
static constexpr int width_ = 640;
static constexpr int height_ = 480;
static constexpr int nChannel_ = 3;
static constexpr int nBytesOfChannel_ = 1;
static constexpr int fps_ = 30;
static constexpr const char* codec_ = "YUYV";

static uint8_t data_[height_ * width_ * nChannel_ * nBytesOfChannel_];


// Capture_CVクラスのインスタンスで正常にキャプチャできること
TEST(TS_Capture_Camera, TC01)
{
    auto cap = CvCapture(dev_, width_, height_, nChannel_, nBytesOfChannel_, fps_, codec_, is_dbg_);

    const char* display = "TS_Capture/TC01";
    cv::namedWindow(display, cv::WINDOW_AUTOSIZE);

    auto capDataObject = new CaptureDataObject(data_, nBytesOfChannel_, height_ * width_ * nChannel_ * nBytesOfChannel_);

    while(true)
    {
        auto ret = cap.Capture(capDataObject);
        if (!ret) break;

        auto mat = cv::Mat(height_, width_, CV_8UC3, (void *)data_);
        cv::imshow(display, mat);

        auto key = cv::waitKey(2);
        if(key == 27) break;
    }

    delete capDataObject;

    cv::destroyAllWindows();
}

// MultiThreadCaptureControllerクラスのインスタンスで正常にキャプチャできること (マルチスレッド)
TEST(TS_Capture_Camera, TC02)
{
    auto controller = MultiThreadCaptureController(new CvCapture(dev_, width_, height_, nChannel_, nBytesOfChannel_, fps_, codec_), is_cap_delete_, is_dbg_);

    const char* display = "TS_Capture/TC02";
    cv::namedWindow(display, cv::WINDOW_AUTOSIZE);

    controller.Setup();
    controller.StartCapture();

    while(true)
    {
        auto readResult = controller.Read();
        auto capDataObject = std::get<0>(readResult);
        auto img = capDataObject->Data;
        if (img == nullptr)
        {
            break;
        }

        auto mat = cv::Mat(height_, width_, CV_8UC3, (void *)img);
        cv::imshow(display, mat);

        auto key = cv::waitKey(2);
        if(key == 27) break;
    }

    controller.FinishCapture();
    cv::destroyAllWindows();
}

