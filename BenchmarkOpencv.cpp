#include "BenchmarkOpencv.h"

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"  
#include "opencv/cv.hpp"

#include "thread_pool.hpp"

#include <vector>
#include <string>

#include <thread>

#include<memory>
#include<functional>

#include "VideoUtil.h"


using namespace std;
using namespace cv;

double calculateRunTime(std::function<void()> pVoidFunc) {
	double t = (double)getTickCount();

	if (!pVoidFunc) return -1.0f;

	pVoidFunc();

	return  ((double)getTickCount() - t) / getTickFrequency();

}

void task1(){
	for (int i = 1; i <= 30; i++)
		readVideoSeekPos("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\42.mp4", 50);
}

void task2(){
	for (int i = 1; i <= 30; i++)
		readVideoSeekPos("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\43.mp4", 50);
}

void task3(){
	for (int i = 1; i <= 30; i++)
		readVideoSeekPos("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\44.mp4", 50);
}

void task4(){
	for (int i = 1; i <= 30; i++)
		readVideoSeekPos("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\45.mp4", 50);
}

void testCreateFileCapture2Threads(){
	thread thread1(task1);
	thread thread2(task2);

	thread1.join();
	thread2.join();
	//task1();
	//task2();
}

/*
���Դ��ж�120����Ƶ�ļ���4���̲߳��ж�30����Ƶ�ļ���Ч��
���Խ����ʾ��(��λ����)�����߳�Ч������Ҳ��20%���ң�Ч�����Ǻ�����
1���̴߳��ж�120���ļ���ʱ490.958
4���̲߳��ж�120���ļ���ʱ398.265
*/
void testReadVideo4ThreadsPrallel(){
	thread thread1(task1);
	thread thread2(task2);
	thread thread3(task3);
	thread thread4(task4);

	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
	
}


void testReadVideoSerial() {
	task1();
	task2();
	task3();
	task4();
}

void testCreateFileCapture(){
	//CvCapture *capture = cvCreateFileCapture("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\10.mp4");
	//CvCapture *capture1 = cvCreateFileCapture("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\11.mp4");
	//CvCapture *capture2 = cvCreateFileCapture("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\12.mp4");
	//CvCapture *capture3 = cvCreateFileCapture("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\13.mp4");
	//CvCapture *capture4 = cvCreateFileCapture("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\14.mp4");

	//VideoCapture vc("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\10.mp4");
	//VideoCapture vc1("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\11.mp4");
	//VideoCapture vc2("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\12.mp4");
	//VideoCapture vc3("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\13.mp4");
	//VideoCapture vc4("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\14.mp4");

	readVideoEveryXFramesInNthPart("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\42.mp4", 1, 1, 100);
	//vector<Mat> vImg;
	//VideoUtil::readVideo("42.mp4", "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08", 100, vImg);
}
/*
��������readVideoEveryXFramesInNthPart
���ã�����Ƶ�ļ����ֳ�N��NSegments���ж�ȡ�� �˺�����ȡ��nth�Σ� ÿ����Ƶ�����ǰ���֡���ж�ȡ������ÿ��X��֡��ȡ
Ĭ�϶�ȡ������Ƶ
*/
void readVideoEveryXFramesInNthPart(const char* fileFullPath, int NSegments, int nth, int period) {

	CvCapture *capture = cvCreateFileCapture(fileFullPath);
	int numFrames = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
	//��Ƶ�ļ���Ϊ�գ�ֱ�ӷ���-1
	if (capture == NULL)
	{
		return;
	}
	IplImage *frame;
	int nStart = int(numFrames / NSegments)*(nth - 1);
	int nEnd = int(numFrames / NSegments)*nth - 1;
	int nPos = nStart;
	while (nPos <= nEnd )
	{
		if (!cvGrabFrame(capture))
			break;
		//0-based index of the frame to be decoded/captured next
		//double t = (double)getTickCount();
		cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, nPos);
		//cout << int((nPos + 1)*1.0 / numFrames * 60 * 1000) << endl;
		//cvSetCaptureProperty(capture, CV_CAP_PROP_POS_MSEC, int((nPos + 1)*1.0 / numFrames * 60 * 1000));
		//cout << "cvSetCaptureProperty��ʱ:" << ((double)getTickCount() - t) / getTickFrequency() << endl;
		//t = (double)getTickCount();
		frame = cvRetrieveFrame(capture);
		//frame = cvQueryFrame(capture);
		//cout << "cvRetrieveFrame��ʱ:" << ((double)getTickCount() - t) / getTickFrequency() << endl;
		if (!frame)
			break;
		//cvNamedWindow("showImg");
		//cvShowImage("showImg", frame);
		//cvWaitKey(0);
		nPos += period;
		//nPos++;
	}
	cvReleaseCapture(&capture);

}

void test_read_video_thread_pool()
{
	thread_pool tp(-1);
	std::function<void()> pVoidFunc = std::bind(readVideoEveryXFramesInNthPart, "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\00.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc1 = std::bind(readVideoEveryXFramesInNthPart, "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\01.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc2 = std::bind(readVideoEveryXFramesInNthPart, "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\02.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc3 = std::bind(readVideoEveryXFramesInNthPart, "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\03.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc4 = std::bind(readVideoEveryXFramesInNthPart, "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\04.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc5 = std::bind(readVideoEveryXFramesInNthPart, "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\05.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc6 = std::bind(readVideoEveryXFramesInNthPart, "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\06.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc7 = std::bind(readVideoEveryXFramesInNthPart, "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\07.mp4", 1, 1, 50);


	tp.submit(pVoidFunc);
	tp.submit(pVoidFunc1);
	tp.submit(pVoidFunc2);
	tp.submit(pVoidFunc3);
	tp.submit(pVoidFunc4);
	tp.submit(pVoidFunc5);
	tp.submit(pVoidFunc6);
	tp.submit(pVoidFunc7);

	std::this_thread::sleep_for(std::chrono::seconds(13));

	//return 0;
}

void test_read_video()
{
	readVideoEveryXFramesInNthPart("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\00.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\01.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\02.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\03.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\04.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\05.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\06.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09\\07.mp4", 1, 1);
}

/*
�������ܲ��Ժ��������Զ�ȡ��Ƶ�ļ����������ʱ�Ĳ���������Ƶ��ȡ������ʹ��period������ʾÿ������֡��ȡһ֡
ÿ������֡����ʹ�õ���capture.set(CV_CAP_PROP_POS_FRAMES, nPos)
ͨ�����Է������capture.set�ĺ�ʱ�� capture.retrieve��ȡ����ͼ���50��100�����ϣ�����ת����Ƶ�����֡�ĺ�ʱ��
������Ƶǰ��֡�ĺ�ʱ���࣬��Ҫԭ�������ڵ���Ƶ���ǲ���֡����ѹ���ģ���֡�ض�����Ƶ֡��Ҫ�ӵ�һ֡��ʼ���뵽Ҫ
��ת��֡����ʱ�����������+ƫ�Ƶķ�ʽ
��ȡһ��60s��ʵ��Ƶ�ļ��ĺ�ʱ�������(��λ����)��(��testReadVideoSeekPos����)
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.0546961
capture.retrieve(frame)��ʱ:0.00535765
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.0985311
capture.retrieve(frame)��ʱ:0.0025655
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.124484
capture.retrieve(frame)��ʱ:0.00211133
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.155517
capture.retrieve(frame)��ʱ:0.00222209
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.056346
capture.retrieve(frame)��ʱ:0.00259971
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.093262
capture.retrieve(frame)��ʱ:0.00214768
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.115724
capture.retrieve(frame)��ʱ:0.00222594
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.159582
capture.retrieve(frame)��ʱ:0.00225417
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.0718288
capture.retrieve(frame)��ʱ:0.00218617
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.114055
capture.retrieve(frame)��ʱ:0.00224262
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.123639
capture.retrieve(frame)��ʱ:0.00266942
capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:0.151518
capture.retrieve(frame)��ʱ:0.00216821

ͬʱ��������SeekPos�ķ�ʽ��Ҫ�����ӵ�1֡��λ��Ҫ��ȡ��֡��λ�ã�����ڼ����֡period�Ƚ�С������£������������
��֡��ͷ����β�ķ�ʽЧ�ʸߣ�����μӺ���readVideoEveryFrame������ͬһ����Ƶ��readVideoSeekPosÿ��100֡��ȡ
һ֡�ĺ�ʱ��1.6�����ң���readVideoEveryFrame��֡��ͷ����β�ĺ�ʱ��3.4s��������periodС��40֡�Ļ���
readVideoEveryFrame��Ч�ʷ�������readVideoSeekPos
*/
int readVideoSeekPos(const char* filePath, int period) {
	//��μ��
	if (NULL == filePath)
		return -1;
	VideoCapture capture;
	capture.open(filePath); //����Ƶ�ļ�
	//��ȡ��Ƶ�ļ��ܹ��ж���֡
	int numFrames = (int)capture.get(CV_CAP_PROP_FRAME_COUNT);
	//��Ƶ�ļ���Ϊ�գ�ֱ�ӷ���-1
	if (!capture.isOpened())
	{
		return  -1;
	}
	Mat frame;
	int nPos = period;
	while (nPos <= numFrames && capture.grab())
	{
		//double t = (double)getTickCount();
		capture.set(CV_CAP_PROP_POS_FRAMES, nPos);
		//cout << "capture.set(CV_CAP_PROP_POS_FRAMES)��ʱ:" << ((double)getTickCount() - t) / getTickFrequency() << endl;


		//t = (double)getTickCount();
		bool bRes = capture.retrieve(frame);
		//cout << "capture.retrieve(frame)��ʱ:" << ((double)getTickCount() - t) / getTickFrequency() << endl;
		//ʵ���ҵĳ������еĹ��������ǻ�ȡ���յ�֡������Ӹ�����
		if (!bRes)
			break;
		nPos += period;
	}

	capture.release();
	return numFrames;
}

int readVideoEveryFrame(const char* filePath, int period) {
	//��μ��
	if (NULL == filePath)
		return -1;
	VideoCapture capture;
	capture.open(filePath); //����Ƶ�ļ�
	//��ȡ��Ƶ�ļ��ܹ��ж���֡
	int numFrames = (int)capture.get(CV_CAP_PROP_FRAME_COUNT);
	//��Ƶ�ļ���Ϊ�գ�ֱ�ӷ���-1
	if (!capture.isOpened())
	{
		return  -1;
	}
	Mat frame;
	int nPos = period;
	while (nPos <= numFrames && capture.grab())
	{
		double t = (double)getTickCount();
		bool bRes = capture.retrieve(frame);
		if (!bRes)
			break;
		nPos ++;
		if (nPos%period == 0) {
			//ÿ��period֡��ȡһ��֡
		}

	}

	capture.release();
	return numFrames;
}

void testReadVideoSeekPos() {
	char *fileName = "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\42.mp4";
	int period = 100;
	readVideoSeekPos(fileName, period);

}

void testReadVideoEveryFrame() {
	char *fileName = "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\42.mp4";
	int period = 100;
	readVideoEveryFrame(fileName, period);
}