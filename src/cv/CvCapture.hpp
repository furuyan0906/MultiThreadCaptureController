#ifndef  H__CAPTURE_CV__H
#define  H__CAPTURE_CV__H

#include  <string>
#include  <memory>
#include  <tuple>
#include  <cstdint>
#include  "opencv2/opencv.hpp"
#include  "opencv2/videoio.hpp"
#include  "common/ICapturable.hpp"

class CvCapture : public ICapturable 
{
	private:
		bool isDebug_;

		int dev_;
		int width_;
		int height_;
		int nChannel_;
		int nBytesOfChannel_;
		int type_;
		int fps_;
		std::string codec_;
		std::string filename_;

		cv::VideoCapture cap_;
		static constexpr int nBuffer_ = 1;

		bool init(
			int dev,
			int width, int height, int nChannel, int nBytesOfChannel,
			int fps, const std::string codec
		);

		bool init(
			const std::string& filename
		);

		bool capture(uint8_t* const image, int image_width, int image_height, int num_channel);

		std::tuple<int, int, int, int> get_size(void);

		void __dbgPrint(int line, const char* str, const char* fmt, ...);

	public:
		CvCapture(
			int dev,
			int width, int height, int num_channel, int nbytes,
			int fps, const char codec[4],
	        bool isDebug = false
		);

		CvCapture(
			const std::string& filename,
	        bool isDebug = false
		);
	
		~CvCapture();

		bool Capture(const CaptureDataObject *) override;

        uint64_t GetNBytes() override;

        uint64_t GetLength() override;
};

#endif  /* H__CAPTURE_CV__H */

