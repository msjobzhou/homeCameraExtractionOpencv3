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

bool FrameDiffDetect::FrameDetectResult(const vector<Mat> &imgFrame) {
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
		return -1;
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
		/*
		//��ʾͼƬ
		cout << "show contours" << endl;
		/// Show in a window
		imshow("Contours", drawing);
		waitKey();
		*/

		
		//д����Ƶ

		writer << threeChannelFrameOut;
		//imshow("video", threeChannelFrameOut);
		if (cvWaitKey(20) == 27)
		{
			break;
		}
		
		lastFrame = frame;

	}
	writer.release();
}
