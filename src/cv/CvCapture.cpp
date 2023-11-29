#include  <iostream>
#include  <memory>
#include  <cstdint>
#include  <cstring>
#include  "CvCapture.hpp"


/* ----- Public ----- */

CvCapture::CvCapture(
	int dev,
	int width, int height, int nChannel, int nBytesOfChannel,
	int fps, const char codec[4],
    bool isDebug
)
	: isDebug_(isDebug), 
	  dev_(dev), width_(width), height_(height), nChannel_(nChannel), nBytesOfChannel_(nBytesOfChannel),
	  fps_(fps), codec_(std::string(codec)),
	  filename_(std::string{}),
	  cap_(cv::VideoCapture())
{
	auto ret = init(dev_, width_, height_, nChannel_, nBytesOfChannel_, fps_, codec_);
	if (!ret)
	{
		if (cap_.isOpened()) cap_.release();
	}
}

CvCapture::CvCapture(
	const std::string& filename,
    bool isDebug
)
	: isDebug_(isDebug), 
	  dev_(-1), width_(-1), height_(-1), nChannel_(-1), nBytesOfChannel_(-1),
	  fps_(-1), codec_(std::string{}),
	  filename_(filename),
	  cap_(cv::VideoCapture())
{
	auto ret = init(filename_);
	if (!ret)
	{
		if (cap_.isOpened()) cap_.release();
	}
}

CvCapture::~CvCapture()
{
	if (cap_.isOpened()) cap_.release();
}


bool CvCapture::Capture(const CaptureDataObject * captureDataObject)
{
    auto data = (uint8_t*)(captureDataObject->Data);
    return this->capture(data, width_, height_, nChannel_);
}

uint64_t CvCapture::GetNBytes()
{
    return sizeof(std::uint8_t);
}

uint64_t CvCapture::GetLength()
{
    return width_ * height_;
}

/* ----- Private ----- */

bool CvCapture::init(int dev, int width, int height, int channel, int nBytes, int fps, const std::string codec)
{
	cap_.open(dev, cv::CAP_V4L2);
	if (!cap_.isOpened()) 
    {
        std::cout << "fail to open" << std::endl;
        return false;
    }

	auto isSuccess = true;

	// set user-specified parameters
	isSuccess &= cap_.set(cv::CAP_PROP_FRAME_WIDTH, width);
	isSuccess &= cap_.set(cv::CAP_PROP_FRAME_HEIGHT, height);
	isSuccess &= cap_.set(cv::CAP_PROP_FPS, fps);
	isSuccess &= cap_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc(codec[0], codec[1], codec[2], codec[3]));
	cap_.set(cv::CAP_PROP_BUFFERSIZE, nBuffer_);

	return isSuccess;
}

bool CvCapture::init(const std::string& filename)
{
	cap_.open(filename);
	if (!cap_.isOpened()) 
    {
        std::cout << "fail to open" << std::endl;
        return false;
    }

	auto isSuccess = true;

	// get user-specified parameters
	width_ = (int)cap_.get(cv::CAP_PROP_FRAME_WIDTH);
	isSuccess &= (width_ != -1);

	height_ = (int)cap_.get(cv::CAP_PROP_FRAME_HEIGHT);
	isSuccess &= (height_ != -1);

	nChannel_ = 3;
    nBytesOfChannel_ = sizeof(uint8_t);

	fps_ = (int)cap_.get(cv::CAP_PROP_FPS);
	isSuccess &= (fps_ != -1);

	codec_ = std::string("mp4v");

	cap_.set(cv::CAP_PROP_BUFFERSIZE, nBuffer_);

	return isSuccess;
}

bool CvCapture::capture(uint8_t* const image, int width, int height, int nChannel)
{
	assert(width == width_);
	assert(height == height_);
	assert(nChannel_ == nChannel);

	if (!cap_.isOpened())
	{
		return false;
	}

	auto mat = cv::Mat(height, width, CV_8UC3, (void *)image);
	auto ret = cap_.read(mat);

	return ret;
}

std::tuple<int, int, int, int> CvCapture::get_size(void)
{
	return std::tuple<int, int, int, int>(width_, height_, nChannel_, nBytesOfChannel_);
}


