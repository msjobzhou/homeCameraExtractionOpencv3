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
#include "BenchmarkSqlite3.hpp"

#include <iostream> 
#include <string> 
#include <locale> 
#include <codecvt> 
#include <fstream> 

#include "codeConvert.hpp"
#include "SingleConsumerSingleProducer.hpp"
#include <atomic>
#include <vector>

using namespace std;

vector<string> gVecFolder;
vector<string> gVecFile;
Database *g_pdb=NULL;
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
	char *pDbName = "C:\\Users\\chao\\gitRepo\\learnPython\\testdb_cpp.db";
	g_pdb = new Database(pDbName);
	char *path1 = "E:\\周晓董视频备份\\客厅墙上";
	char *path2 = "E:\\周晓董视频备份\\客厅电视柜上";
	// 插入操作第一个参数ID是个自增字段
	g_pdb->m_IDtable.insert(NULL, gbk_to_utf8(path1));
	g_pdb->m_IDtable.insert(NULL, gbk_to_utf8(path2));

	vector<vector<string> > results;
	vector<string> oneRow;

	//查询InitialDirectory这个数据库表，并根据得到的初始文件夹目录，并在初始目录下进一步去遍历得到其下有文件的
	//子文件夹，并将其插入ScanDirectory的数据库表
	g_pdb->m_IDtable.query(results);

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
			//插入数据库
			g_pdb->m_SDtable.insert(NULL, atoi(id.c_str()), gbk_to_utf8(*vFolder));
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


	//查询ScanDirectory这个数据库表，并根据得到的文件夹目录，进一步去遍历得到其下的文件名
	//并将其插入ScanFile的数据库表
	g_pdb->m_SDtable.query(results);

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
			cout << *iterFile << endl;
			//插入数据库
			g_pdb->m_SFtable.insert(NULL, atoi(id.c_str()), gbk_to_utf8(*iterFile));
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


SingleConsumerSingleProducer<vector<string>> videoReadSubmit2ThreadPool;

void ProducerTask_ReadVideoFromDB() // 生产者任务
{
	//从sqlite中一次读10个未处理的文件，具体的读取方法是初次读取时利用SQL语句找到未处理文件ID最小的那个
	//然后从这个ID开始读取时刻10个处理的文件，并将这次读取中最大的ID的后一个作为下次读取10个文件的起始ID，
	//并以此类推循环读下去
	while (true) {
		if (g_bOver)
			break;
		//用户触发的中断如果发生在这里,即另一个线程将g_bOver设置成true,同时这个时候cpu执行切换到了消费
		//者线程且恰好消费的商品数等于生产的商品数,这个时候就会导致消费者线程退出,但生产者线程还会继续走下
		//去,并且放到缓冲区的商品将没有消费者线程再处理了,不过这种场景也没啥太大的问题,逻辑上符合用户触发
		//程序终止的逻辑设定,本身用户触发程序终止的意思就是在还没有处理完待处理的视频文件就终止

		//从数据库中读取待处理的文件,如果读取到未处理的视频文件的记录
		//先将已经生成的商品数量加1,然后再放到缓冲区,如果读取到的记录数小于10个,则设置g_bOver为true
		//生产者线程自行终止
		videoReadSubmit2ThreadPool.ProduceItem(i);

		//如果数据库中读取的记录个数小于10个，则设置g_bOver为true
	}
}

void ConsumerTask_SubmitVideoFile2ThreadPool() // 消费者任务
{
	thread_pool tpVideoProcess(-1);
	//消费者在缓冲区非空且线程池thread_pool中的task任务个数小于等于5的情况下，
	//从缓冲区中取到待处理的视频文件（以slot为单位，每次10个文件），
	//执行thread_pool类的submit函数提交视频处理任务
	while (true) { 
		if (g_bOver&&消费的商品数等于生产的商品数)
			break;

		while (消费的商品数小于生产的商品数&&tpVideoProcess.taskNumber <= 5) {
			item = videoReadSubmit2ThreadPool.ConsumeItem(); // 消费一个产品.
			//提交thread_pool
		}
		//休眠1秒，等待thread_pool处理视频
		sleep(1)
	}
		
}


void homeCameraExtractionMainLoop() {
	InitVideoFileDatabase();

	std::thread producer(ProducerTask_ReadVideoFromDB); // 创建生产者线程.
	std::thread consumer(ConsumerTask_SubmitVideoFile2ThreadPool); // 创建消费之线程.
	producer.join();
	consumer.join();


	if (g_pdb != NULL)
		delete g_pdb;
}