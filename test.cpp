//���ڲ��Դ������ʱ�ļ�����������ɵĴ���ſ��Ը��õ�ִ��
//#pragma warning (disable��4996)
#include "test.h"
#include "VideoUtil.h"
#include "FolderUtil.h"
#include "FrameDetect.h"
#include <stdio.h>
#include <stdlib.h>
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

#include "codeConvert.hpp"
#include "SingleConsumerSingleProducer.hpp"

using namespace cv;
vector<string> gVecFolder;
vector<string> gVecFile;

void test_zhongwen() {
	std::wcout << "User-preferred locale setting is " << std::locale("").name().c_str() << '\n';
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
		std::locale::global(std::locale(""));
		std::wofstream ofs(L"d:\\testW.txt");
		std::ofstream ofs2("d:\\testW111.txt");
		wcout.imbue(std::locale(""));
		ofs.imbue(std::locale(""));
		if (!ofs) {
			std::cout << "open file failed" << endl;
		}
		wstring_convert<codecvt<wchar_t, char, mbstate_t>> gbk_cvt(new codecvt<wchar_t, char, mbstate_t>("chs"));
		ofs2 << gbk_cvt.to_bytes(str);
		
		ofs << wideStr;
		wcout << wideStr;
	}
}

void test_FolderHasFiles() {
	char *path1 = "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08";
	cout << FolderUtil::FolderHasFiles(path1) << endl;
}

void tfh_sqlite(string& path) {
	if (FolderUtil::isFolder(path.c_str())) {
		if (FolderUtil::FolderHasFiles(path))
			gVecFolder.push_back(path);
	}
}
void lfh_sqlite(string& file) {
	gVecFile.push_back(file);
}

void test_Database_class() {
	char *pDbName = "C:\\Users\\chao\\gitRepo\\learnPython\\testdb_cpp.db";
	Database *pdb = new Database(pDbName);
	char *path1 = "E:\\��������Ƶ����\\����ǽ��";
	char *path2 = "E:\\��������Ƶ����\\�������ӹ���";
	// ���������һ������ID�Ǹ������ֶ�
	pdb->m_IDtable.insert(NULL, gbk_to_utf8(path1));
	pdb->m_IDtable.insert(NULL, gbk_to_utf8(path2));
	
	vector<vector<string> > results;
	vector<string> oneRow;

	//��ѯInitialDirectory������ݿ�������ݵõ��ĳ�ʼ�ļ���Ŀ¼�����ڳ�ʼĿ¼�½�һ��ȥ�����õ��������ļ���
	//���ļ��У����������ScanDirectory�����ݿ��
	pdb->m_IDtable.query(results);

	vector<vector<string> >::iterator v = results.begin();
	while (v != results.end()) {
		oneRow = *v;
		/*for (int i = 0; i < oneRow.size();i++)
			cout << " | " << utf8_to_gbk(oneRow.at(i));
		cout << endl;*/
		//�Ӳ�ѯ��ÿ�м�¼�ĵڶ����еõ� ��ʼɨ��Ŀ¼����һ���еõ�ID
		string id = utf8_to_gbk(oneRow.at(0));
		string initialPath = utf8_to_gbk(oneRow.at(1));
		//���˳�ʼĿ¼�µ����ļ��У������ļ��ģ����뵽sqlite�����е�ScanDirectory��
		traverseFolder_handler2 tfh = tfh_sqlite;
		FolderUtil::traverseFolderBFS(initialPath, tfh);

		vector<string>::iterator vFolder = gVecFolder.begin();
		while (vFolder != gVecFolder.end()) {
			cout << *vFolder << endl;
			//�������ݿ�
			pdb->m_SDtable.insert(NULL, atoi(id.c_str()), gbk_to_utf8(*vFolder));
			vFolder++;
		}
		gVecFolder.clear();
		vector<string>(gVecFolder).swap(gVecFolder);
		v++;
	}
	oneRow.clear();
	vector<string>(oneRow).swap(oneRow);
	results.clear();
	vector<vector<string>>(results).swap(results);


	//��ѯScanDirectory������ݿ�������ݵõ����ļ���Ŀ¼����һ��ȥ�����õ����µ��ļ���
	//���������ScanFile�����ݿ��
	pdb->m_SDtable.query(results);

	v = results.begin();
	while (v != results.end()) {
		oneRow = *v;
		/*for (int i = 0; i < oneRow.size();i++)
		cout << " | " << utf8_to_gbk(oneRow.at(i));
		cout << endl;*/
		//�Ӳ�ѯ��ÿ�м�¼�ĵ������еõ��ļ���Ŀ¼����һ���еõ�ID
		string id = utf8_to_gbk(oneRow.at(0));
		string scanPath = utf8_to_gbk(oneRow.at(2));
		//����scanĿ¼�µ��ļ������뵽sqlite�����е�ScanFile��
		listFile_handler lfh = lfh_sqlite;
		FolderUtil::listFiles(scanPath, lfh);

		vector<string>::iterator iterFile = gVecFile.begin();
		while (iterFile != gVecFile.end()) {
			cout << *iterFile << endl;
			//�������ݿ�
			pdb->m_SFtable.insert(NULL, atoi(id.c_str()), gbk_to_utf8(*iterFile));
			iterFile++;
		}
		gVecFile.clear();
		vector<string>(gVecFile).swap(gVecFile);
		v++;
	}
	oneRow.clear();
	vector<string>(oneRow).swap(oneRow);
	results.clear();
	vector<vector<string>>(results).swap(results);

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
	FrameDiffDetect *pFd = new FrameDiffDetect();
	pFd->FrameDetectResultSaveVideo(vImg);
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
	FrameDiffDetect *pFd = new FrameDiffDetect();
	pFd->FrameDetectResultSaveVideo(vImg);
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
void printString(string& s) {
	cout << s << endl;
}
void test_traverseFolderBFS() {
	traverseFolder_handler2 tfh = printString;
	string path = "E:\\��������Ƶ����\\�������ӹ���";
	FolderUtil::traverseFolderBFS(path, tfh);
}
SingleConsumerSingleProducer<int> ss;
int kItemsToProduce = 10;
void ProducerTask() // ����������
{
	for (int i = 1; i <= kItemsToProduce; ++i) {
		// std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Produce the " << i << "^th item..." << std::endl;
		ss.ProduceItem(i); // ѭ������ kItemsToProduce ����Ʒ.
	}
}

void ConsumerTask() // ����������
{
	static int cnt = 0;
	while (1) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		int item = ss.ConsumeItem(); // ����һ����Ʒ.
		std::cout << "Consume the " << item << "^th item" << std::endl;
		if (++cnt == kItemsToProduce) break; // �����Ʒ���Ѹ���Ϊ kItemsToProduce, ���˳�.
	}
}

void test_SingleConsumerSingleProducer_class() {
	std::thread producer(ProducerTask); // �����������߳�.
	std::thread consumer(ConsumerTask); // ��������֮�߳�.
	producer.join();
	consumer.join();
}