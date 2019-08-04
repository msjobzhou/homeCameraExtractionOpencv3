#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

class FrameDetect {
public:
	virtual bool FrameDetectResult(const vector<Mat> &imgFrame) = 0;
};

class FrameDiffDetect : public FrameDetect {
public:
	bool FrameDetectResult(const vector<Mat> &imgFrame);
	bool FrameDetectResultSaveVideo(const vector<Mat> &imgFrame);
};