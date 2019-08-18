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
		readVideoSeekPos("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\42.mp4", 50);
}

void task2(){
	for (int i = 1; i <= 30; i++)
		readVideoSeekPos("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\43.mp4", 50);
}

void task3(){
	for (int i = 1; i <= 30; i++)
		readVideoSeekPos("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\44.mp4", 50);
}

void task4(){
	for (int i = 1; i <= 30; i++)
		readVideoSeekPos("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\45.mp4", 50);
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
测试串行读120个视频文件和4个线程并行读30个视频文件的效率
测试结果显示：(单位是秒)，多线程效率提升也就20%左右，效果不是很明显
1个线程串行读120次文件耗时490.958
4个线程并行读120次文件耗时398.265
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
	//CvCapture *capture = cvCreateFileCapture("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\10.mp4");
	//CvCapture *capture1 = cvCreateFileCapture("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\11.mp4");
	//CvCapture *capture2 = cvCreateFileCapture("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\12.mp4");
	//CvCapture *capture3 = cvCreateFileCapture("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\13.mp4");
	//CvCapture *capture4 = cvCreateFileCapture("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\14.mp4");

	//VideoCapture vc("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\10.mp4");
	//VideoCapture vc1("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\11.mp4");
	//VideoCapture vc2("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\12.mp4");
	//VideoCapture vc3("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\13.mp4");
	//VideoCapture vc4("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\14.mp4");

	readVideoEveryXFramesInNthPart("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\42.mp4", 1, 1, 100);
	//vector<Mat> vImg;
	//VideoUtil::readVideo("42.mp4", "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08", 100, vImg);
}
/*
函数名：readVideoEveryXFramesInNthPart
作用：将视频文件均分成N段NSegments进行读取， 此函数读取第nth段， 每段视频并不是挨个帧进行读取，而是每隔X个帧读取
默认读取整个视频
*/
void readVideoEveryXFramesInNthPart(const char* fileFullPath, int NSegments, int nth, int period) {

	CvCapture *capture = cvCreateFileCapture(fileFullPath);
	int numFrames = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
	//视频文件打开为空，直接返回-1
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
		//cout << "cvSetCaptureProperty耗时:" << ((double)getTickCount() - t) / getTickFrequency() << endl;
		//t = (double)getTickCount();
		frame = cvRetrieveFrame(capture);
		//frame = cvQueryFrame(capture);
		//cout << "cvRetrieveFrame耗时:" << ((double)getTickCount() - t) / getTickFrequency() << endl;
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
	std::function<void()> pVoidFunc = std::bind(readVideoEveryXFramesInNthPart, "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\00.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc1 = std::bind(readVideoEveryXFramesInNthPart, "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\01.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc2 = std::bind(readVideoEveryXFramesInNthPart, "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\02.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc3 = std::bind(readVideoEveryXFramesInNthPart, "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\03.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc4 = std::bind(readVideoEveryXFramesInNthPart, "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\04.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc5 = std::bind(readVideoEveryXFramesInNthPart, "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\05.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc6 = std::bind(readVideoEveryXFramesInNthPart, "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\06.mp4", 1, 1, 50);
	std::function<void()> pVoidFunc7 = std::bind(readVideoEveryXFramesInNthPart, "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\07.mp4", 1, 1, 50);


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
	readVideoEveryXFramesInNthPart("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\00.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\01.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\02.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\03.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\04.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\05.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\06.mp4", 1, 1);
	readVideoEveryXFramesInNthPart("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09\\07.mp4", 1, 1);
}

/*
基础性能测试函数，测试读取视频文件过程中最耗时的部分整个视频读取过程中使用period参数表示每隔多少帧读取一帧
每隔多少帧具体使用的是capture.set(CV_CAP_PROP_POS_FRAMES, nPos)
通过测试发现这个capture.set的耗时是 capture.retrieve获取具体图像的50到100倍以上，且跳转到视频后面的帧的耗时，
比在视频前面帧的耗时更多，主要原因是现在的视频都是采用帧关联压缩的，跳帧特定的视频帧需要从第一帧开始解码到要
跳转的帧，耗时不能类比数组+偏移的方式
读取一个60s真实视频文件的耗时情况如下(单位是秒)：(见testReadVideoSeekPos函数)
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.0546961
capture.retrieve(frame)耗时:0.00535765
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.0985311
capture.retrieve(frame)耗时:0.0025655
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.124484
capture.retrieve(frame)耗时:0.00211133
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.155517
capture.retrieve(frame)耗时:0.00222209
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.056346
capture.retrieve(frame)耗时:0.00259971
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.093262
capture.retrieve(frame)耗时:0.00214768
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.115724
capture.retrieve(frame)耗时:0.00222594
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.159582
capture.retrieve(frame)耗时:0.00225417
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.0718288
capture.retrieve(frame)耗时:0.00218617
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.114055
capture.retrieve(frame)耗时:0.00224262
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.123639
capture.retrieve(frame)耗时:0.00266942
capture.set(CV_CAP_PROP_POS_FRAMES)耗时:0.151518
capture.retrieve(frame)耗时:0.00216821

同时由于这种SeekPos的方式需要反复从第1帧定位到要获取的帧的位置，因此在间隔的帧period比较小的情况下，还不如从那种
逐帧从头读到尾的方式效率高，具体参加函数readVideoEveryFrame，对于同一个视频，readVideoSeekPos每隔100帧读取
一帧的耗时是1.6秒左右，而readVideoEveryFrame逐帧从头读到尾的耗时是3.4s，因此如果period小于40帧的话，
readVideoEveryFrame的效率反而高于readVideoSeekPos
*/
int readVideoSeekPos(const char* filePath, int period) {
	//入参检查
	if (NULL == filePath)
		return -1;
	VideoCapture capture;
	capture.open(filePath); //打开视频文件
	//获取视频文件总共有多少帧
	int numFrames = (int)capture.get(CV_CAP_PROP_FRAME_COUNT);
	//视频文件打开为空，直接返回-1
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
		//cout << "capture.set(CV_CAP_PROP_POS_FRAMES)耗时:" << ((double)getTickCount() - t) / getTickFrequency() << endl;


		//t = (double)getTickCount();
		bool bRes = capture.retrieve(frame);
		//cout << "capture.retrieve(frame)耗时:" << ((double)getTickCount() - t) / getTickFrequency() << endl;
		//实际我的程序运行的过程中总是获取到空的帧，这里加个保护
		if (!bRes)
			break;
		nPos += period;
	}

	capture.release();
	return numFrames;
}

int readVideoEveryFrame(const char* filePath, int period) {
	//入参检查
	if (NULL == filePath)
		return -1;
	VideoCapture capture;
	capture.open(filePath); //打开视频文件
	//获取视频文件总共有多少帧
	int numFrames = (int)capture.get(CV_CAP_PROP_FRAME_COUNT);
	//视频文件打开为空，直接返回-1
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
			//每隔period帧获取一个帧
		}

	}

	capture.release();
	return numFrames;
}

void testReadVideoSeekPos() {
	char *fileName = "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\42.mp4";
	int period = 100;
	readVideoSeekPos(fileName, period);

}

void testReadVideoEveryFrame() {
	char *fileName = "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\42.mp4";
	int period = 100;
	readVideoEveryFrame(fileName, period);
}