#include "homeCameraExtractionMain.h"
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


#include <iostream> 
#include <string> 
#include <locale> 
#include <codecvt> 
#include <fstream> 

#include "codeConvert.h"
#include "SingleConsumerSingleProducer.hpp"
#include <atomic>
#include <vector>
#include<functional>

#include <assert.h>
#include <signal.h>


using namespace std;

vector<string> gVecFolder;
vector<string> gVecFile;
//Database *g_pdb=NULL;
char *g_pDbName = "C:\\Users\\chao\\gitRepo\\learnPython\\testdb_cpp.db";
std::atomic<bool> g_bOver=false;

void tfh_sqlite(string& path) {
	if (FolderUtil::isFolder(path.c_str())) {
		if (FolderUtil::FolderHasFiles(path))
			gVecFolder.push_back(path);
	}
}
void lfh_sqlite(string& file) {
	gVecFile.push_back(file);
}

void InitVideoFileDatabase() {
	//设置sqlite为多线程串行模式
	int rc = sqlite3_config(SQLITE_CONFIG_SERIALIZED);
	cout << "sqlite3_config_result :" << rc << endl;
	//char *pDbName = "C:\\Users\\chao\\gitRepo\\learnPython\\testdb_cpp.db";
	Database *pdb = new Database(g_pDbName);
	char *path1 = "E:\\周晓董视频备份已处理文件";
	//char *path2 = "E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\09";
	// 插入操作第一个参数ID是个自增字段
	pdb->m_IDtable.insert(NULL, gbk_to_utf8(path1));
	//pdb->m_IDtable.insert(NULL, gbk_to_utf8(path2));

	vector<vector<string> > results;
	vector<string> oneRow;

	//查询InitialDirectory这个数据库表，并根据得到的未处理的初始文件夹目录，并在初始目录下进一步去遍历得到其下有文件的
	//子文件夹，并将其插入ScanDirectory的数据库表
	string sqlQuery = string("select * from InitialDirectory where HandledMark != 1");
	pdb->m_IDtable.query(results, sqlQuery);

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
			//cout << *vFolder << endl;
			//插入数据库
			pdb->m_SDtable.insert(NULL, atoi(id.c_str()), gbk_to_utf8(*vFolder));
			vFolder++;
		}
		//标记成对应的ID记录已经处理（写入到表ScanDirectory）
		pdb->m_IDtable.update_bHandledMark(atoi(id.c_str()), true);
		gVecFolder.clear();
		vector<string>(gVecFolder).swap(gVecFolder);
		v++;
	}
	oneRow.clear();
	vector<string>(oneRow).swap(oneRow);
	results.clear();
	vector<vector<string>>(results).swap(results);


	//查询ScanDirectory这个数据库表，并根据得到未处理的文件夹目录，进一步去遍历得到其下的文件名
	//并将其插入ScanFile的数据库表
	string sqlQuery2 = string("select * from ScanDirectory where HandledMark != 1");
	pdb->m_SDtable.query(results, sqlQuery2);

	v = results.begin();
	while (v != results.end()) {
		oneRow = *v;
		/*for (int i = 0; i < oneRow.size();i++)
		cout << " | " << utf8_to_gbk(oneRow.at(i));
		cout << endl;*/
		//从查询的每行记录的第三列中得到文件夹目录，第一列中得到ID
		string id = utf8_to_gbk(oneRow.at(0));
		string scanPath = utf8_to_gbk(oneRow.at(2));
		//将此scan目录下的文件名加入到sqlite数据中的ScanFile表
		listFile_handler lfh = lfh_sqlite;
		FolderUtil::listFiles(scanPath, lfh);

		vector<string>::iterator iterFile = gVecFile.begin();
		while (iterFile != gVecFile.end()) {
			//cout << *iterFile << endl;
			//插入数据库
			pdb->m_SFtable.insert(NULL, atoi(id.c_str()), gbk_to_utf8(*iterFile));
			iterFile++;
		}
		//标记成对应的ID记录已经处理（写入到表ScanDirectory）
		pdb->m_SDtable.update_bHandledMark(atoi(id.c_str()), true);
		gVecFile.clear();
		vector<string>(gVecFile).swap(gVecFile);
		v++;
	}
	oneRow.clear();
	vector<string>(oneRow).swap(oneRow);
	results.clear();
	vector<vector<string>>(results).swap(results);
	if (pdb != NULL)
		delete pdb;
}


SingleConsumerSingleProducer<vector<string>> gConsumerProducer_videoReadSubmit2ThreadPool;
atomic<int> gItemNumProduced = 0;
int gItemNumConsumed = 0;
int gVideoNumToHandled = 0;
atomic<int> gVideoNumAlreadyHandled = 0;

void ProducerTask_ReadVideoFromDB() // 生产者任务
{
	Database *pdb = new Database(g_pDbName);
	//得出要处理的视频总数
	string sqlQueryTotalNum = string("select count(*) from ScanFile where DeleteMark is null;");
	vector<vector<string> > resultsTotalNum;
	vector<string> oneRow;
	pdb->m_SFtable.query(resultsTotalNum, sqlQueryTotalNum);
	if (resultsTotalNum.empty()) {
		cout <<"要处理的视频总数为0" << endl;
		return;
	}
	oneRow = resultsTotalNum.at(0);
	string strTotalNum = oneRow.at(0);
	gVideoNumToHandled = atoi(strTotalNum.c_str());
	cout <<"需要处理的视频总数：" << gVideoNumToHandled << endl;
	//从sqlite中一次读10个未处理的文件，具体的读取方法是初次读取时利用SQL语句找到未处理文件ID最小的那个
	//然后从这个ID开始读取时刻10个处理的文件，并将这次读取中最大的ID的后一个作为下次读取10个文件的起始ID，
	//并以此类推循环读下去
	int initialID = 1;
	char chInitialID[16] = {0};
	while (true) {
		if (g_bOver)
			break;
		//用户触发的中断如果发生在这里,即另一个线程将g_bOver设置成true,同时这个时候cpu执行切换到了消费
		//者线程且恰好消费的商品数等于生产的商品数,这个时候就会导致消费者线程退出,但生产者线程还会继续走下
		//去,并且放到缓冲区的商品将没有消费者线程再处理了,不过这种场景也没啥太大的问题,逻辑上符合用户触发
		//程序终止的逻辑设定,本身用户触发程序终止的意思就是在还没有处理完待处理的视频文件就终止

		//从数据库中读取待处理的文件,如果读取到有未处理的视频文件的记录
		vector<vector<string> > results;
		vector<string> oneRow;
		vector<string> vecProduceItem;
		_itoa(initialID, chInitialID, 10);
		string sqlQuery = string("select * from ScanFile where ID >= ") + string(chInitialID)
			+ string(" and DeleteMark is null limit 10;");
		//cout << "sqlQuery: " << sqlQuery << endl;
		pdb->m_SFtable.query(results, sqlQuery);
		if (!results.empty()) {
			vector<vector<string> >::iterator v = results.begin();
			vector<vector<string> > resultsDirectory;
			vector<string> oneRowDirectory;
			string scanDirectoryID;
			string fileName;
			string idPrimaryKey;
			while (v != results.end()) {
				oneRow = *v;

				//从查询的每行记录的第二列中得到 ScanDirectoryID，第三列中得到FileName
				scanDirectoryID = utf8_to_gbk(oneRow.at(1));
				fileName = utf8_to_gbk(oneRow.at(2));
				idPrimaryKey = utf8_to_gbk(oneRow.at(0));
				//根据scanDirectoryID从ScanDirectory表中读取到目录，
				//并将此目录和FileName组合成一个完整的绝对路径
				string sqlQueryDirectory = string("select * from ScanDirectory where ID = ")
					+ oneRow.at(1);
				//cout << "sqlQueryDirectory: " << sqlQueryDirectory << endl;
				pdb->m_SDtable.query(resultsDirectory, sqlQueryDirectory);
				if (resultsDirectory.empty()) {
					cout << sqlQueryDirectory << "result is empty" << endl;
					break;
				}
				//断言这个查询记录只有1条
				assert(resultsDirectory.size() == 1);
				oneRowDirectory = resultsDirectory.at(0);
				//ScanDirectory表的第三列是path
				string absoluteFilePath = oneRowDirectory.at(2) + '\\' + fileName;
				//将完整绝对路径文件名放到一个vector<string>中
				vecProduceItem.push_back(absoluteFilePath);
				
				resultsDirectory.clear();
				vector<vector<string>>(resultsDirectory).swap(resultsDirectory);
				oneRowDirectory.clear();
				vector<string>(oneRowDirectory).swap(oneRowDirectory);
				
				v++;
			}
			initialID = atoi(idPrimaryKey.c_str()) + 1;
		}
		//先将已经生成的商品数量加1,然后再放到缓冲区,如果读取到的记录数小于10个,则设置g_bOver为true
		//生产者线程自行终止
		gItemNumProduced++;
		gConsumerProducer_videoReadSubmit2ThreadPool.ProduceItem(vecProduceItem);

		//如果数据库中读取的记录个数小于10个，则设置g_bOver为true
		if (results.size() < 10) {
			g_bOver = true;
			cout << "g_bOver is set to true" << endl;
		}
			

		vecProduceItem.clear();
		vector<string>(vecProduceItem).swap(vecProduceItem);
		oneRow.clear();
		vector<string>(oneRow).swap(oneRow);
		results.clear();
		vector<vector<string>>(results).swap(results);

	}

	if (pdb != NULL)
		delete pdb;
}


void videoProceed(vector<string> vecVideoAbsolutePath) {
	//传入的字符串是utf8格式,FolderUtil和videoUtil中的函数字符串路径全是gbk的,这里涉及到字符串格式转换
	Database *pdb = new Database(g_pDbName);
	vector<string>::iterator v = vecVideoAbsolutePath.begin();
	vector<vector<string> > resultsDirectory;
	vector<string> oneRowDirectory;
	vector<vector<string> > resultsFile;
	vector<string> oneRowFile;
	while (v != vecVideoAbsolutePath.end()) {
		vector<Mat> vImg;
		string videoAbsolutePath=*v;
		string videoAbsolutePathGBK = utf8_to_gbk(videoAbsolutePath);
		char fileName[100] = {0};
		char filePath[255] = {0};
		FolderUtil::getFolderAndFilename((char*)videoAbsolutePathGBK.c_str(), filePath, fileName);
		int period = 50;
		//cout << "filePath:" << filePath << endl;
		//cout << "fileName:" << fileName << endl;
		VideoUtil::readVideo(fileName, filePath, period, vImg);
		//去掉路径中末尾的斜杠
		if (filePath[strlen(filePath) - 1] == '\\')
			filePath[strlen(filePath) - 1] = '\0';
		//先从ScanDirectory表中根据filePath找到ID,sqlite3数据库中存储的字符串格式是utf-8
		string sqlQueryDirectory = string("select * from ScanDirectory where Path = ")
			+ "\'" + gbk_to_utf8(filePath) + "\'";
		//cout << "sqlQueryDirectory: " << sqlQueryDirectory << endl;
		pdb->m_SDtable.query(resultsDirectory, sqlQueryDirectory); 
		//cout << "resultsDirectory.size(): " << resultsDirectory.size() << endl;
		if (resultsDirectory.empty()) {
			cout << sqlQueryDirectory << "result is empty" << endl;
			break;
		}
		assert(resultsDirectory.size() == 1);
		oneRowDirectory = resultsDirectory.at(0);
		//ScanDirectory表的第一列是ID
		string ScanDirectoryID = oneRowDirectory.at(0);
		//根据ScanDirectoryID和fileName从ScanFile表中找到对应的记录的ID
		string sqlQueryFile = string("select * from ScanFile where ScanDirectoryID = ")
			+ ScanDirectoryID + " and FileName = " + "\'" + gbk_to_utf8(fileName) + "\'";
		//cout << "sqlQueryFile: " << sqlQueryFile << endl;
		pdb->m_SFtable.query(resultsFile, sqlQueryFile);
		//cout << "resultsFile.size(): " << resultsFile.size() << endl;
		if (resultsFile.empty()) {
			cout << sqlQueryFile << "result is empty" << endl;
			break;
		}
		assert(resultsFile.size() == 1);
		oneRowFile = resultsFile.at(0);
		string id = oneRowFile.at(0);

		//将处理的视频结果写入sqlite数据库
		FrameDiffDetect *pFd = new FrameDiffDetect();
		bool bResult = pFd->FrameDetectResult(vImg);
		//cout << "pFd->FrameDetectResult(vImg):" << bResult << endl;
		pdb->m_SFtable.update_bDeleteMark(atoi(id.c_str()), bResult);
		gVideoNumAlreadyHandled++;
		//释放vector的内存空间，防止内存泄漏
		resultsDirectory.clear();
		vector<vector<string>>(resultsDirectory).swap(resultsDirectory);
		oneRowDirectory.clear();
		vector<string>(oneRowDirectory).swap(oneRowDirectory);
		resultsFile.clear();
		vector<vector<string>>(resultsFile).swap(resultsFile);
		oneRowFile.clear();
		vector<string>(oneRowFile).swap(oneRowFile);
		//释放vector申请的内存空间，防止内存泄漏
		vImg.clear();
		vector<Mat>(vImg).swap(vImg);
		v++;
	}
	
	

	//释放内存
	vecVideoAbsolutePath.clear();
	vector<string>(vecVideoAbsolutePath).swap(vecVideoAbsolutePath);
	if (pdb != NULL)
		delete pdb;
}

void ConsumerTask_SubmitVideoFile2ThreadPool() // 消费者任务
{
	thread_pool tpVideoProcess(-1);
	//消费者在缓冲区非空且线程池thread_pool中的task任务个数小于等于5的情况下，
	//从缓冲区中取到待处理的视频文件（以slot为单位，每次10个文件），
	//执行thread_pool类的submit函数提交视频处理任务
	while (true) { 
		if (g_bOver && (gItemNumConsumed == gItemNumProduced))
			break;

		while ((gItemNumConsumed < gItemNumProduced) && tpVideoProcess.workQueueSize() <= 5) {
			vector<string> vecConsumedItem = gConsumerProducer_videoReadSubmit2ThreadPool.ConsumeItem(); // 消费一个产品.
			
			//vector<string>::iterator v = vecConsumedItem.begin();
			//cout << "vector<string> vecConsumedItem" << endl;
			//while (v != vecConsumedItem.end()) {
			//	cout << *v << endl;
			//	v++;
			//}

			gItemNumConsumed++;
			//提交thread_pool
			//std::cout << "queueSize:" << tpVideoProcess.workQueueSize() << "tp.submit ...\n";
			tpVideoProcess.submit(std::bind(videoProceed, vecConsumedItem));
		}
		//休眠2秒，等待thread_pool处理视频
		std::this_thread::sleep_for(std::chrono::seconds(15));
	}

	//消费者线程每隔5s定时判断threadPool线程池的任务队列是否为空，
	//如果为空，在等待60s之后（给写入数据库视频处理结果的线程预留写入的机会），结束整个流程

	while (1) {
		std::this_thread::sleep_for(std::chrono::seconds(5));
		if (tpVideoProcess.workQueueSize() == 0) {
			break;
		}
	}
	cout << "start waiting for 60s" << endl;
	std::this_thread::sleep_for(std::chrono::seconds(60));
		
}
void test_videoProceed()
{
	vector<string> vecVideoAbsolutePath;
	vecVideoAbsolutePath.push_back(gbk_to_utf8("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\42.mp4"));
	vecVideoAbsolutePath.push_back(gbk_to_utf8("E:\\周晓董视频备份\\客厅墙上\\2018-01-16\\08\\43.mp4"));
	videoProceed(vecVideoAbsolutePath);
}

void printVideoProceedProgress() {
	cout << "待处理视频总数: " << gVideoNumToHandled << " 已处理个数: " \
		<< gVideoNumAlreadyHandled << endl;
}

//捕获信号 CTRL+C
void sighandler(int signum)
{
	g_bOver = true;
	cout << "用户中断程序g_bOver is set to true" << endl;
}

void homeCameraExtractionMainLoop() {
	//注册中断处理函数
	signal(SIGINT, sighandler);

	InitVideoFileDatabase();
	Timer t;
	//每60s打印一次进展
	t.StartTimer(60000, std::bind(printVideoProceedProgress));
	std::thread producer(ProducerTask_ReadVideoFromDB); // 创建生产者线程.
	std::thread consumer(ConsumerTask_SubmitVideoFile2ThreadPool); // 创建消费之线程.
	producer.join();
	consumer.join();
	std::cout << "try to expire timer!" << std::endl;
	t.Expire();

} 