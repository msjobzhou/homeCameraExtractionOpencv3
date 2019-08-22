#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <opencv2/opencv.hpp>
//#include "opencv2/highgui/highgui.hpp"  
//#include "opencv/cv.hpp"

#include <vector>
#include <string>
using namespace std;
using namespace cv;

class VideoUtil {
public :
	static int readVideoSaveImg(const char* fileName, const char* filePath, const char* saveImgPath, int period, vector<string> &vImgPath);

	static int readVideoSeekPos(const char* fileName, const char* filePath, int period, vector<Mat> &vImg);

	static int readVideo(const char* fileName, const char* filePath, int period, vector<Mat> &vImg);
};


