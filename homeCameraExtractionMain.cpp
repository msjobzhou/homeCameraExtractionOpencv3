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
	char *path1 = "E:\\��������Ƶ����\\����ǽ��";
	char *path2 = "E:\\��������Ƶ����\\�������ӹ���";
	// ���������һ������ID�Ǹ������ֶ�
	g_pdb->m_IDtable.insert(NULL, gbk_to_utf8(path1));
	g_pdb->m_IDtable.insert(NULL, gbk_to_utf8(path2));

	vector<vector<string> > results;
	vector<string> oneRow;

	//��ѯInitialDirectory������ݿ�������ݵõ��ĳ�ʼ�ļ���Ŀ¼�����ڳ�ʼĿ¼�½�һ��ȥ�����õ��������ļ���
	//���ļ��У����������ScanDirectory�����ݿ��
	g_pdb->m_IDtable.query(results);

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


	//��ѯScanDirectory������ݿ�������ݵõ����ļ���Ŀ¼����һ��ȥ�����õ����µ��ļ���
	//���������ScanFile�����ݿ��
	g_pdb->m_SDtable.query(results);

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

void ProducerTask_ReadVideoFromDB() // ����������
{
	//��sqlite��һ�ζ�10��δ������ļ�������Ķ�ȡ�����ǳ��ζ�ȡʱ����SQL����ҵ�δ�����ļ�ID��С���Ǹ�
	//Ȼ������ID��ʼ��ȡʱ��10��������ļ���������ζ�ȡ������ID�ĺ�һ����Ϊ�´ζ�ȡ10���ļ�����ʼID��
	//���Դ�����ѭ������ȥ
	while (true) {
		if (g_bOver)
			break;
		//�û��������ж��������������,����һ���߳̽�g_bOver���ó�true,ͬʱ���ʱ��cpuִ���л���������
		//���߳���ǡ�����ѵ���Ʒ��������������Ʒ��,���ʱ��ͻᵼ���������߳��˳�,���������̻߳����������
		//ȥ,���ҷŵ�����������Ʒ��û���������߳��ٴ�����,�������ֳ���Ҳûɶ̫�������,�߼��Ϸ����û�����
		//������ֹ���߼��趨,�����û�����������ֹ����˼�����ڻ�û�д�������������Ƶ�ļ�����ֹ

		//�����ݿ��ж�ȡ��������ļ�,�����ȡ��δ�������Ƶ�ļ��ļ�¼
		//�Ƚ��Ѿ����ɵ���Ʒ������1,Ȼ���ٷŵ�������,�����ȡ���ļ�¼��С��10��,������g_bOverΪtrue
		//�������߳�������ֹ
		videoReadSubmit2ThreadPool.ProduceItem(i);

		//������ݿ��ж�ȡ�ļ�¼����С��10����������g_bOverΪtrue
	}
}

void ConsumerTask_SubmitVideoFile2ThreadPool() // ����������
{
	thread_pool tpVideoProcess(-1);
	//�������ڻ������ǿ����̳߳�thread_pool�е�task�������С�ڵ���5������£�
	//�ӻ�������ȡ�����������Ƶ�ļ�����slotΪ��λ��ÿ��10���ļ�����
	//ִ��thread_pool���submit�����ύ��Ƶ��������
	while (true) { 
		if (g_bOver&&���ѵ���Ʒ��������������Ʒ��)
			break;

		while (���ѵ���Ʒ��С����������Ʒ��&&tpVideoProcess.taskNumber <= 5) {
			item = videoReadSubmit2ThreadPool.ConsumeItem(); // ����һ����Ʒ.
			//�ύthread_pool
		}
		//����1�룬�ȴ�thread_pool������Ƶ
		sleep(1)
	}
		
}


void homeCameraExtractionMainLoop() {
	InitVideoFileDatabase();

	std::thread producer(ProducerTask_ReadVideoFromDB); // �����������߳�.
	std::thread consumer(ConsumerTask_SubmitVideoFile2ThreadPool); // ��������֮�߳�.
	producer.join();
	consumer.join();


	if (g_pdb != NULL)
		delete g_pdb;
}