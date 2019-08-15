
#define _CRT_SECURE_NO_WARNINGS
#include "VideoUtil.h"
//#include "cv.h"
//#include "highgui.h"
#include <string>
#include<stdio.h>
#include<stdlib.h>
#include<direct.h>
#include<io.h>  


#include <vector>
//#include "opencv2/highgui/highgui.hpp"  
#include "FolderUtil.h"

using namespace std;
using namespace cv;
/*
�������ã�readvideoSaveImg ������Ƶ��֡����-1ʱ��ʾ��Ƶ�ļ���Ч, 0��ʾ��֡����
        �������ץȡ֡���ұ���ͼ���ļ���ָ���Ĵ���·���������ļ�������vector vImgPath�У�������ʹ��
		��Ҫ�ر�ע����ǣ��ⲿʹ�ó���Ҫ�����ͷŵ�vImgPath�д洢��string����
���������fileName ��Ҫ��ȡ��Ƶ�ļ���
		filePath ��Ҫ��ȡ��Ƶ�ļ�·�����������·��������"\"��β�����������л����ļ���ǰ��һ��"\"
		period ֡�����ÿ������֡ȡ����һ�Ž�ͼ
		saveImgPath ����Ƶ��ץȡ��ͼ���ŵ�·��
		vImgPath �Ѵ���Ƶ�ж�ȡ��ͼ��ȫ��������vector��������������
*/
int VideoUtil::readVideoSaveImg(const char* fileName, const char* filePath, const char* saveImgPath, int period, vector<string> &vImgPath) {
	//int period;
	int count = 1; //�ļ���ſ�ʼ����ֵ
	
	string fileFullPath = "";
	fileFullPath = fileFullPath + filePath + "\\" + fileName;
	VideoCapture capture;
	capture.open(fileFullPath.c_str()); //����Ƶ�ļ�
	//��ȡ��Ƶ�ļ��ܹ��ж���֡
	int numFrames = (int)capture.get(CV_CAP_PROP_FRAME_COUNT);
	//��Ƶ�ļ���Ϊ�գ�ֱ�ӷ���-1
	if (!capture.isOpened())
	{
		return  -1;
	}
	//printf("totalFrameNum:%d",numFrames);
	Mat frame;
	string strSaveImgPath="";
	int nPos = period;
	//����Ƶ��ץȡ��ͼ�񣬱�����ļ���
	char chFilename[128];
	//while (1)
	while (nPos <= numFrames && capture.grab())
	{
		
		capture.set(CV_CAP_PROP_POS_FRAMES, nPos);
		bool bRes = capture.retrieve(frame);
		//ʵ���ҵĳ������еĹ��������Ǳ���յ�ͼ���ļ�������Ӹ�����
		if (!bRes)
			break;
		//����Ƶ��ץȡ��ͼ�񣬱�����ļ���:��Ƶ�ļ�����+_img_�ۼ�����
		sprintf(chFilename, "%s_img_%d.jpg", fileName, count++);
		string tmpStr;
		//�������Ĳ���saveImgPathΪ�գ���ͼƬ�����ڵ�ǰ���·����
		if (saveImgPath != NULL) {
			tmpStr = saveImgPath;
			int nLen = tmpStr.length();
			//���ݴ���Ĳ���saveImgPath�Ƿ��ԡ�\����β������ͬ�Ĵ���
			if (tmpStr[nLen - 1] != '\\') {
				strSaveImgPath = strSaveImgPath + saveImgPath + "\\" + chFilename;
			} 
			else {
				strSaveImgPath = strSaveImgPath + saveImgPath + chFilename;
			}
			//���Ŀ¼�����ڣ�����㴴�����Զ��庯��
			FolderUtil::mkdirByLevel(saveImgPath);
		}
		else {
			strSaveImgPath = strSaveImgPath + chFilename;
		}
		Mat dstGray;
		cvtColor(frame, dstGray, CV_BGR2GRAY);
		imwrite(strSaveImgPath.c_str(), dstGray);

		//���ļ����Ƽ��뵽vector��
		vImgPath.push_back(strSaveImgPath);

		nPos += period;

		//�ͷ�string�ڴ�
		strSaveImgPath.clear();
		string(strSaveImgPath).swap(strSaveImgPath);
		strSaveImgPath = "";
	}
	capture.release();
	return numFrames;
}


/*
�������ã�readVideo ������Ƶ��֡����-1ʱ��ʾ��Ƶ�ļ���Ч, 0��ʾ��֡���������ұ����ļ���ָ����·��
        �������ץȡ֡���ұ���ͼ���ļ���ָ����vector�ڴ������������ʹ��
		��Ҫ�ر�ע�����vector��ָ����ڴ���ͼ���ⲿʹ�ó���Ҫ�����ͷŵ�
���������fileName ��Ҫ��ȡ��Ƶ�ļ���
filePath ��Ҫ��ȡ��Ƶ�ļ�·�����������·��������"\"��β�����������л����ļ���ǰ��һ��"\"
period ֡�����ÿ������֡ȡ����һ�Ž�ͼ
vImg �Ѵ���Ƶ�ж�ȡ��ͼ��ȫ������һ�ݱ�����vector��������������
*/
int VideoUtil::readVideo(const char* fileName, const char* filePath, int period, vector<Mat> &vImg) {
	//��μ��
	if ((NULL == fileName) || (NULL == filePath))
		return -1;
	
	//int period;
	int count = 1; //�ļ���ſ�ʼ����ֵ

	string fileFullPath = "";
	if (filePath[strlen(filePath) - 1] != '\\')
		fileFullPath = fileFullPath + filePath + "\\" + fileName;
	else 
		fileFullPath = fileFullPath + filePath + fileName;
	VideoCapture capture;
	capture.open(fileFullPath.c_str()); //����Ƶ�ļ�
	//��ȡ��Ƶ�ļ��ܹ��ж���֡
	int numFrames = (int)capture.get(CV_CAP_PROP_FRAME_COUNT);
	//��Ƶ�ļ���Ϊ�գ�ֱ�ӷ���-1
	if (!capture.isOpened())
	{
		return  -1;
	}
	//printf("totalFrameNum:%d",numFrames);
	Mat frame;
	//string strSaveImgPath = "";
	int nPos = period;
	//����Ƶ��ץȡ��ͼ�񣬱�����ļ���
	char chFilename[128];
	//while (1)
	while (nPos <= numFrames && capture.grab())
	{
		capture.set(CV_CAP_PROP_POS_FRAMES, nPos);
		//����cvRetrieveFrame�����ɺ���cvGrabFrame ץȡ��ͼ���ָ�롣���ص�ͼ�񲻿��Ա��û��ͷŻ����޸ġ�
		bool bRes = capture.retrieve(frame);
		//ʵ���ҵĳ������еĹ��������ǻ�ȡ���յ�֡������Ӹ�����
		if (!bRes)
			break;
		Mat dstGray;
		cvtColor(frame, dstGray, CV_BGR2GRAY);
		vImg.push_back(dstGray);
		nPos += period;
	}

	capture.release();
	return numFrames;
}