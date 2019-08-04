#include "FrameDetect.h"
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

Mat convertTo3Channels(const Mat& binImg)
{
    Mat three_channel = Mat::zeros(binImg.rows,binImg.cols,CV_8UC3);
    vector<Mat> channels;
    for (int i=0;i<3;i++)
    {
        channels.push_back(binImg);
    }
    merge(channels,three_channel);
    return three_channel;
}
/*
函数作用：检测从视频提取出来的一系列帧，相邻之间的帧经过帧间差的计算-》二值化-》腐蚀（去噪）之后，查找看看有没有轮廓
面积大于300的，如果有则认为，这段时间内的视频有较大的变化，并返回true，否则返回false。同时此函数还会将面积大于300的帧间
轮廓用长方形圈起来，并保存到一个视频文件中
说明：这个函数中用到的一些诸如300之类的魔鬼数字都是经验值，此函数主要的目的也是测试用，真正使用的函数是FrameDetectResult
*/
bool FrameDiffDetect::FrameDetectResultSaveVideo(const vector<Mat> &imgFrame) {
	int count = imgFrame.size();
	RNG rng(12345);
	if (count <= 1) {
		cerr << "the count of imgFrame is less than 2" << endl;
		return false;
	}
	
	Mat lastFrame=imgFrame[0], frame;
	Mat frameDiff, frameDiffThresh, frameDiffThreshOpen;

	//写入视频相关操作
	VideoWriter writer;
	double fps = 12.0;
	Mat frameOut;
	Mat threeChannelFrameOut;
	//输出灰度视频
	bool isColor = 1;
	//CV_FOURCC('M', 'P', '4', '2')视频格式占用的空间最小
	writer.open("D:\\tmp_comp_img\\result1.avi", CV_FOURCC('M', 'P', '4', '2'), fps, lastFrame.size(), isColor);
	//输出视频的分辨率
	//cout<<lastFrame.size()<<endl;
	if (!writer.isOpened()) {
		std::cout << "Error!Video File is not open..." << endl;
		return false;
	}
	//

	for (int i = 1; i < count; i++) {
		frame = imgFrame[i];
		//打印图片的通道数
		//cout << "image channels: " << frame.channels() << endl;
		//图像差分
		absdiff(lastFrame, frame, frameDiff);
		//图像二值化
		threshold(frameDiff, frameDiffThresh, 25, 255, cv::THRESH_BINARY);
		//形态学开运算
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
		//morphologyEx(frameDiffThresh, frameDiffThreshOpen, MORPH_DILATE, element);
		//下面注释掉的这个语句是测试代码，主要是看看如果注释掉 erode这行，会产生啥效果
		//frameDiffThresh.copyTo(frameDiffThreshOpen);
		erode(frameDiffThresh, frameDiffThreshOpen, element, cv::Point(-1, -1), 1);

		//查找轮廓
		/// Find contours
		vector<vector<Point> > contours;
		//vector<Vec4i> hierarchy;
		//findContours(frameDiffThreshOpen, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
		findContours(frameDiffThreshOpen, contours, cv::noArray(), RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		vector<Rect> boundRect(contours.size());
		/// Draw contours
		Mat drawing = Mat::zeros(frameDiffThreshOpen.size(), CV_8UC3);
		for (size_t i = 0; i< contours.size(); i++)
		{
			boundRect[i] = boundingRect(Mat(contours[i]));
			Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
			//drawContours(drawing, contours, (int)i, color, 1, 8, hierarchy, 0);
			drawContours(drawing, contours, (int)i, color, 1, 8, cv::noArray(), 0);
			//轮廓面积小的不在外面画长方形圈起来了
			if (contourArea(contours[i]) < 300)
				continue;
			rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(0, 255, 0), 1, 8, 0);
			//在当前帧上检测出来的轮廓上画上框圈起来
			//frame.copyTo(frameOut);
			//rectangle(frameOut, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(255), 1, 8, 0);
			//单通道转换成3通道
			frame.copyTo(frameOut);
			threeChannelFrameOut = convertTo3Channels(frameOut);
			rectangle(threeChannelFrameOut, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(0, 255, 0), 3, 8, 0);

		}
				
		//写入视频

		writer << threeChannelFrameOut;
		
		lastFrame = frame;

	}
	writer.release();
}
/*
函数作用：检测从视频提取出来的一系列帧，相邻之间的帧经过帧间差的计算-》二值化-》腐蚀（去噪）之后，查找看看有没有轮廓
面积大于300的，如果有则认为，这段时间内的视频有较大的变化，并返回true，否则返回false。
此函数里面的300等值来自于FrameDetectResultSaveVideo中调测的经验值
*/
bool FrameDiffDetect::FrameDetectResult(const vector<Mat> &imgFrame) {
	int count = imgFrame.size();
	RNG rng(12345);
	if (count <= 1) {
		cerr << "the count of imgFrame is less than 2" << endl;
		return false;
	}

	Mat lastFrame = imgFrame[0], frame;
	Mat frameDiff, frameDiffThresh, frameDiffThreshOpen;

	Mat frameOut;

	int frameDiffCount_ContourAreaGreaterThan300 = 0;
	for (int i = 1; i < count; i++) {
		frame = imgFrame[i];
		//图像差分
		absdiff(lastFrame, frame, frameDiff);
		//图像二值化
		threshold(frameDiff, frameDiffThresh, 25, 255, cv::THRESH_BINARY);
		//腐蚀
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
		//morphologyEx(frameDiffThresh, frameDiffThreshOpen, MORPH_DILATE, element);
		//下面注释掉的这个语句是测试代码，主要是看看如果注释掉 erode这行，会产生啥效果
		//frameDiffThresh.copyTo(frameDiffThreshOpen);
		erode(frameDiffThresh, frameDiffThreshOpen, element, cv::Point(-1, -1), 1);

		//查找轮廓
		/// Find contours
		vector<vector<Point> > contours;
		findContours(frameDiffThreshOpen, contours, cv::noArray(), RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		vector<Rect> boundRect(contours.size());
		for (size_t i = 0; i < contours.size(); i++)
		{
			boundRect[i] = boundingRect(Mat(contours[i]));
			//每个帧间差中如果有一个轮廓面积大于300，则把计数加1
			if (contourArea(contours[i]) > 300) {
				frameDiffCount_ContourAreaGreaterThan300++;
				break;
			}
		}
		lastFrame = frame;
	}
	//如果有3个帧间差，其有面积大于300的轮廓，则认为这个视频画面变化较大，并返回true 
	if (frameDiffCount_ContourAreaGreaterThan300 >= 3)
		return true;

	return false;
}