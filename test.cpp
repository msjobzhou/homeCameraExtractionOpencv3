//���ڲ��Դ������ʱ�ļ�����������ɵĴ���ſ��Ը��õ�ִ��

#include "test.h"
#include "VideoUtil.h"
#include "FolderUtil.h"
#include "FrameDetect.h"
#include <stdio.h>
#include <sqlite3.h>
#include<opencv2\opencv.hpp>

#include "Database.h"
#include "ReadWriteThread.h"
#include "Timer.hpp"
#include "thread_pool.hpp"
#include "BenchmarkSqlite3.hpp"

#include <iostream> 
#include <string> 
#include <locale> 
#include <codecvt> 
#include <fstream> 



using namespace cv;


void test_zhongwen() {
	std::wstring str = L"123,����˭���Ұ����㵺��";
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	std::string narrowStr = conv.to_bytes(str);
	{
		std::ofstream ofs("d:\\test.txt");
		if (!ofs) {
			std::cout << "open file failed" << endl;
		}

		ofs << narrowStr;
	}
	std::wstring wideStr = conv.from_bytes(narrowStr);
	{
		std::locale::global(std::locale("chs"));
		std::wofstream ofs(L"d:\\testW.txt");
		if (!ofs) {
			std::cout << "open file failed" << endl;
		}
		ofs << wideStr;
	}
}
void test_Database_class() {
	char *pDbName = "C:\\Users\\chao\\gitRepo\\learnPython\\testdb_cpp.db";
	Database *pdb = new Database(pDbName);
	// ���������һ������ID�Ǹ������ֶ�
	pdb->m_IDtable.insert(NULL, "E:\\��������Ƶ����\\����ǽ��");
	pdb->m_IDtable.insert(NULL, "E:\\��������Ƶ����\\�������ӹ���");
	
	vector<vector<string> > results;

	vector<string> oneRow;

	pdb->m_IDtable.query(results);

	vector<vector<string> >::iterator v = results.begin();
	while (v != results.end()) {
		oneRow = *v;
		for (int i = 0; i < oneRow.size();i++)
			cout << " | " << oneRow.at(i) ;
		cout << endl;
		v++;
	}
}


static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	for (i = 0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

void test_sqlite(){
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *dbName = "C:\\Users\\chao\\gitRepo\\learnPython\\sqlitZh.db";
	
	rc = sqlite3_open(dbName, &db);
	if (rc){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}
	char *sql = "select * from student";
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	sqlite3_close(db);
	
}



static std::string strName[] = { "������", "����", "���", "��߸", "ľ߸", "������" };
int sqlite3_exec_callback(void *data, int nColumn, char **colValues, char **colNames)
{
	for (int i = 0; i < nColumn; i++)
	{
		printf("%s\t", colValues[i]);
	}
	printf("\n");
	return 0;
}
int test_sqlite_zhongwen()
{
	sqlite3 *conn = NULL;
	char *err_msg = NULL;
	char sql[200] = "";
	//�����ݿ�,��������;
	if (sqlite3_open("test_for_cpp.db", &conn) != SQLITE_OK)
	{
		printf("�޷���\n");
	}
	//ִ��SQL������һ�ű�;
	sprintf(sql, "CREATE TABLE test_for_cpp(id int,name varchar(20),age int)");
	if (sqlite3_exec(conn, sql, NULL, NULL, &err_msg) != SQLITE_OK)
	{
		std::string strErrMsg(err_msg);
		std::string::size_type pos = 0;
		pos = strErrMsg.find("already exists");
		if (std::string::npos != pos)
		{
			//���ݱ��Ѵ��ڣ�ɾ����������;
			sprintf(sql, "delete from test_for_cpp");
			if (sqlite3_exec(conn, sql, NULL, NULL, &err_msg) != SQLITE_OK)
			{
				printf("����ʧ�ܣ��������:%s", err_msg);
				return -1;
			}
		}
		else
		{
			printf("����ʧ�ܣ��������:%s", err_msg);
			return -1;
		}
	}
	//��������;
	int nColumn = sizeof(strName) / sizeof(strName[0]);
	for (int index = 0; index < nColumn; index++)
	{
		sprintf(sql, "INSERT INTO test_for_cpp (id, name, age) VALUES (%d, '%s', %d)", index, strName[index].c_str(), 20 + index);
		if (sqlite3_exec(conn, sql, NULL, NULL, &err_msg) != SQLITE_OK)
		{
			printf("����ʧ�ܣ��������: %s", err_msg);
			return -1;
		}
	}
	// ��ѯ;
	sprintf(sql, "SELECT * FROM test_for_cpp");
	sqlite3_exec(conn, sql, &sqlite3_exec_callback, 0, &err_msg);
	//�ر�����;
	if (sqlite3_close(conn) != SQLITE_OK)
	{
		printf("�޷��رգ�������룺%s\n", sqlite3_errmsg(conn));
		return -1;
	}
	return 0;
}


void test_FrameDetectResultAndSaveVideo() {
	
	vector<Mat> vImg;
	const char* fileName = "39.mp4";
	const char* filePath = "D:\\test";
	int period = 30;
	VideoUtil::readVideo(fileName, filePath, period, vImg);
	FrameDetect *pFd = new FrameDiffDetect();
	pFd->FrameDetectResult(vImg);
	//�ͷ�vector���ڴ�ռ䣬��ֹ�ڴ�й©
	//�ͷ�vector������ڴ�ռ䣬��ֹ�ڴ�й©
	vImg.clear();
	vector<Mat>(vImg).swap(vImg);
}


void test_FrameDetectResult() {
	vector<Mat> vImg;
	Mat mImg1, mImg2;
	mImg1 = imread("D:\\tmp_comp_img\\14.mp4_img_1.jpg", cv::IMREAD_GRAYSCALE);
	mImg2 = imread("D:\\tmp_comp_img\\14.mp4_img_2.jpg", cv::IMREAD_GRAYSCALE);
	if (mImg1.empty() || mImg2.empty()) {
		cerr << "read img failed" << endl;
	}
	vImg.push_back(mImg1);
	vImg.push_back(mImg2);
	FrameDetect *pFd = new FrameDiffDetect();
	pFd->FrameDetectResult(vImg);
	//�ͷ�vector���ڴ�ռ䣬��ֹ�ڴ�й©
	//�ͷ�vector������ڴ�ռ䣬��ֹ�ڴ�й©
	vImg.clear();
	vector<Mat>(vImg).swap(vImg);
}


void test_readVideoSaveImg() {
	vector<string> vImgPath;
	char *fileName = "14.mp4";
	char *chPath = "D:\\test";
	char *imgSavePath = "D:\\tmp_comp_img\\";
	int nFrameNum;
	nFrameNum = VideoUtil::readVideoSaveImg(fileName, chPath, imgSavePath, 100, vImgPath);
}


void showImg() 
{
    Mat picture = imread("D:\\tmp_comp_img\\00.mp4_img_1.jpg");
	imshow("���Գ���", picture);
	waitKey(0);
}

void test_camera() {
	
	VideoCapture capture(0);
	VideoWriter writer("VideoTest.avi", CV_FOURCC('M', 'J', 'P', 'G'), 25.0, Size(640, 480));
	Mat frame;

	while (capture.isOpened())
	{
		capture >> frame;
		writer << frame;
		imshow("video", frame);
		if (cvWaitKey(20) == 27)
		{
			break;
		}
	}

	
}