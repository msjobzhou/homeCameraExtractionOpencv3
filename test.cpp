//用于测试代码的临时文件，被测试完成的代码才可以更好地执行
//#pragma warning (disable：4996)
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

#include "codeConvert.hpp"


using namespace cv;
vector<string> gVecFolder;

void test_zhongwen() {
	std::wcout << "User-preferred locale setting is " << std::locale("").name().c_str() << '\n';
	std::wstring str = L"123,我是谁？我爱钓鱼岛！";
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

void test_zhongwen2() {

}

void tfh_sqlite(string& path) {
	if (FolderUtil::isFolder(path.c_str())) {
		gVecFolder.push_back(path);
	}
}

void test_Database_class() {
	char *pDbName = "C:\\Users\\chao\\gitRepo\\learnPython\\testdb_cpp.db";
	Database *pdb = new Database(pDbName);
	char *path1 = "E:\\周晓董视频备份\\客厅墙上";
	char *path2 = "E:\\周晓董视频备份\\客厅电视柜上";
	// 插入操作第一个参数ID是个自增字段
	pdb->m_IDtable.insert(NULL, gbk_to_utf8(path1));
	pdb->m_IDtable.insert(NULL, gbk_to_utf8(path2));
	
	vector<vector<string> > results;

	vector<string> oneRow;

	pdb->m_IDtable.query(results);

	vector<vector<string> >::iterator v = results.begin();
	while (v != results.end()) {
		oneRow = *v;
		/*for (int i = 0; i < oneRow.size();i++)
			cout << " | " << utf8_to_gbk(oneRow.at(i));
		cout << endl;*/
		//从查询的每行记录的第二列中得到 初始扫描目录，第一列中得到ID
		string id = utf8_to_gbk(oneRow.at(0));
		string initialPath = utf8_to_gbk(oneRow.at(1));
		//将此初始目录下的子文件夹（含有文件的）加入到sqlite数据中的ScanDirectory表
		traverseFolder_handler2 tfh = tfh_sqlite;
		FolderUtil::traverseFolderBFS(initialPath, tfh);

		vector<string>::iterator vFolder = gVecFolder.begin();
		while (vFolder != gVecFolder.end()) {
			cout << *vFolder << endl;
			vFolder++;
		}

		gVecFolder.clear();
		vector<string>(gVecFolder).swap(gVecFolder);

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



static std::string strName[] = { "土行孙", "哪咤", "杨戬", "金吒", "木吒", "雷震子" };
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
	//打开数据库,创建连接;
	if (sqlite3_open("test_for_cpp.db", &conn) != SQLITE_OK)
	{
		printf("无法打开\n");
	}
	//执行SQL，创建一张表;
	sprintf(sql, "CREATE TABLE test_for_cpp(id int,name varchar(20),age int)");
	if (sqlite3_exec(conn, sql, NULL, NULL, &err_msg) != SQLITE_OK)
	{
		std::string strErrMsg(err_msg);
		std::string::size_type pos = 0;
		pos = strErrMsg.find("already exists");
		if (std::string::npos != pos)
		{
			//数据表已存在，删除表中数据;
			sprintf(sql, "delete from test_for_cpp");
			if (sqlite3_exec(conn, sql, NULL, NULL, &err_msg) != SQLITE_OK)
			{
				printf("操作失败，错误代码:%s", err_msg);
				return -1;
			}
		}
		else
		{
			printf("操作失败，错误代码:%s", err_msg);
			return -1;
		}
	}
	//插入数据;
	int nColumn = sizeof(strName) / sizeof(strName[0]);
	for (int index = 0; index < nColumn; index++)
	{
		sprintf(sql, "INSERT INTO test_for_cpp (id, name, age) VALUES (%d, '%s', %d)", index, strName[index].c_str(), 20 + index);
		if (sqlite3_exec(conn, sql, NULL, NULL, &err_msg) != SQLITE_OK)
		{
			printf("操作失败，错误代码: %s", err_msg);
			return -1;
		}
	}
	// 查询;
	sprintf(sql, "SELECT * FROM test_for_cpp");
	sqlite3_exec(conn, sql, &sqlite3_exec_callback, 0, &err_msg);
	//关闭连接;
	if (sqlite3_close(conn) != SQLITE_OK)
	{
		printf("无法关闭，错误代码：%s\n", sqlite3_errmsg(conn));
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
	//释放vector的内存空间，防止内存泄漏
	//释放vector申请的内存空间，防止内存泄漏
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
	//释放vector的内存空间，防止内存泄漏
	//释放vector申请的内存空间，防止内存泄漏
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
	imshow("测试程序", picture);
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
	string path = "E:\\周晓董视频备份\\客厅电视柜上";
	FolderUtil::traverseFolderBFS(path, tfh);
}