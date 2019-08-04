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
�������ã�������Ƶ��ȡ������һϵ��֡������֮���֡����֡���ļ���-����ֵ��-����ʴ��ȥ�룩֮�󣬲��ҿ�����û������
�������300�ģ����������Ϊ�����ʱ���ڵ���Ƶ�нϴ�ı仯��������true�����򷵻�false��ͬʱ�˺������Ὣ�������300��֡��
�����ó�����Ȧ�����������浽һ����Ƶ�ļ���
˵��������������õ���һЩ����300֮���ħ�����ֶ��Ǿ���ֵ���˺�����Ҫ��Ŀ��Ҳ�ǲ����ã�����ʹ�õĺ�����FrameDetectResult
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

	//д����Ƶ��ز���
	VideoWriter writer;
	double fps = 12.0;
	Mat frameOut;
	Mat threeChannelFrameOut;
	//����Ҷ���Ƶ
	bool isColor = 1;
	//CV_FOURCC('M', 'P', '4', '2')��Ƶ��ʽռ�õĿռ���С
	writer.open("D:\\tmp_comp_img\\result1.avi", CV_FOURCC('M', 'P', '4', '2'), fps, lastFrame.size(), isColor);
	//�����Ƶ�ķֱ���
	//cout<<lastFrame.size()<<endl;
	if (!writer.isOpened()) {
		std::cout << "Error!Video File is not open..." << endl;
		return false;
	}
	//

	for (int i = 1; i < count; i++) {
		frame = imgFrame[i];
		//��ӡͼƬ��ͨ����
		//cout << "image channels: " << frame.channels() << endl;
		//ͼ����
		absdiff(lastFrame, frame, frameDiff);
		//ͼ���ֵ��
		threshold(frameDiff, frameDiffThresh, 25, 255, cv::THRESH_BINARY);
		//��̬ѧ������
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
		//morphologyEx(frameDiffThresh, frameDiffThreshOpen, MORPH_DILATE, element);
		//����ע�͵����������ǲ��Դ��룬��Ҫ�ǿ������ע�͵� erode���У������ɶЧ��
		//frameDiffThresh.copyTo(frameDiffThreshOpen);
		erode(frameDiffThresh, frameDiffThreshOpen, element, cv::Point(-1, -1), 1);

		//��������
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
			//�������С�Ĳ������滭������Ȧ������
			if (contourArea(contours[i]) < 300)
				continue;
			rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(0, 255, 0), 1, 8, 0);
			//�ڵ�ǰ֡�ϼ������������ϻ��Ͽ�Ȧ����
			//frame.copyTo(frameOut);
			//rectangle(frameOut, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(255), 1, 8, 0);
			//��ͨ��ת����3ͨ��
			frame.copyTo(frameOut);
			threeChannelFrameOut = convertTo3Channels(frameOut);
			rectangle(threeChannelFrameOut, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(0, 255, 0), 3, 8, 0);

		}
				
		//д����Ƶ

		writer << threeChannelFrameOut;
		
		lastFrame = frame;

	}
	writer.release();
}
/*
�������ã�������Ƶ��ȡ������һϵ��֡������֮���֡����֡���ļ���-����ֵ��-����ʴ��ȥ�룩֮�󣬲��ҿ�����û������
�������300�ģ����������Ϊ�����ʱ���ڵ���Ƶ�нϴ�ı仯��������true�����򷵻�false��
�˺��������300��ֵ������FrameDetectResultSaveVideo�е���ľ���ֵ
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
		//ͼ����
		absdiff(lastFrame, frame, frameDiff);
		//ͼ���ֵ��
		threshold(frameDiff, frameDiffThresh, 25, 255, cv::THRESH_BINARY);
		//��ʴ
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
		//morphologyEx(frameDiffThresh, frameDiffThreshOpen, MORPH_DILATE, element);
		//����ע�͵����������ǲ��Դ��룬��Ҫ�ǿ������ע�͵� erode���У������ɶЧ��
		//frameDiffThresh.copyTo(frameDiffThreshOpen);
		erode(frameDiffThresh, frameDiffThreshOpen, element, cv::Point(-1, -1), 1);

		//��������
		/// Find contours
		vector<vector<Point> > contours;
		findContours(frameDiffThreshOpen, contours, cv::noArray(), RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		vector<Rect> boundRect(contours.size());
		for (size_t i = 0; i < contours.size(); i++)
		{
			boundRect[i] = boundingRect(Mat(contours[i]));
			//ÿ��֡����������һ�������������300����Ѽ�����1
			if (contourArea(contours[i]) > 300) {
				frameDiffCount_ContourAreaGreaterThan300++;
				break;
			}
		}
		lastFrame = frame;
	}
	//�����3��֡�������������300������������Ϊ�����Ƶ����仯�ϴ󣬲�����true 
	if (frameDiffCount_ContourAreaGreaterThan300 >= 3)
		return true;

	return false;
}