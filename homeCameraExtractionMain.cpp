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
	//����sqliteΪ���̴߳���ģʽ
	int rc = sqlite3_config(SQLITE_CONFIG_SERIALIZED);
	cout << "sqlite3_config_result :" << rc << endl;
	//char *pDbName = "C:\\Users\\chao\\gitRepo\\learnPython\\testdb_cpp.db";
	Database *pdb = new Database(g_pDbName);
	char *path1 = "E:\\��������Ƶ�����Ѵ����ļ�";
	//char *path2 = "E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\09";
	// ���������һ������ID�Ǹ������ֶ�
	pdb->m_IDtable.insert(NULL, gbk_to_utf8(path1));
	//pdb->m_IDtable.insert(NULL, gbk_to_utf8(path2));

	vector<vector<string> > results;
	vector<string> oneRow;

	//��ѯInitialDirectory������ݿ�������ݵõ���δ����ĳ�ʼ�ļ���Ŀ¼�����ڳ�ʼĿ¼�½�һ��ȥ�����õ��������ļ���
	//���ļ��У����������ScanDirectory�����ݿ��
	string sqlQuery = string("select * from InitialDirectory where HandledMark != 1");
	pdb->m_IDtable.query(results, sqlQuery);

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
			//cout << *vFolder << endl;
			//�������ݿ�
			pdb->m_SDtable.insert(NULL, atoi(id.c_str()), gbk_to_utf8(*vFolder));
			vFolder++;
		}
		//��ǳɶ�Ӧ��ID��¼�Ѿ�����д�뵽��ScanDirectory��
		pdb->m_IDtable.update_bHandledMark(atoi(id.c_str()), true);
		gVecFolder.clear();
		vector<string>(gVecFolder).swap(gVecFolder);
		v++;
	}
	oneRow.clear();
	vector<string>(oneRow).swap(oneRow);
	results.clear();
	vector<vector<string>>(results).swap(results);


	//��ѯScanDirectory������ݿ�������ݵõ�δ������ļ���Ŀ¼����һ��ȥ�����õ����µ��ļ���
	//���������ScanFile�����ݿ��
	string sqlQuery2 = string("select * from ScanDirectory where HandledMark != 1");
	pdb->m_SDtable.query(results, sqlQuery2);

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
			//cout << *iterFile << endl;
			//�������ݿ�
			pdb->m_SFtable.insert(NULL, atoi(id.c_str()), gbk_to_utf8(*iterFile));
			iterFile++;
		}
		//��ǳɶ�Ӧ��ID��¼�Ѿ�����д�뵽��ScanDirectory��
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

void ProducerTask_ReadVideoFromDB() // ����������
{
	Database *pdb = new Database(g_pDbName);
	//�ó�Ҫ�������Ƶ����
	string sqlQueryTotalNum = string("select count(*) from ScanFile where DeleteMark is null;");
	vector<vector<string> > resultsTotalNum;
	vector<string> oneRow;
	pdb->m_SFtable.query(resultsTotalNum, sqlQueryTotalNum);
	if (resultsTotalNum.empty()) {
		cout <<"Ҫ�������Ƶ����Ϊ0" << endl;
		return;
	}
	oneRow = resultsTotalNum.at(0);
	string strTotalNum = oneRow.at(0);
	gVideoNumToHandled = atoi(strTotalNum.c_str());
	cout <<"��Ҫ�������Ƶ������" << gVideoNumToHandled << endl;
	//��sqlite��һ�ζ�10��δ������ļ�������Ķ�ȡ�����ǳ��ζ�ȡʱ����SQL����ҵ�δ�����ļ�ID��С���Ǹ�
	//Ȼ������ID��ʼ��ȡʱ��10��������ļ���������ζ�ȡ������ID�ĺ�һ����Ϊ�´ζ�ȡ10���ļ�����ʼID��
	//���Դ�����ѭ������ȥ
	int initialID = 1;
	char chInitialID[16] = {0};
	while (true) {
		if (g_bOver)
			break;
		//�û��������ж��������������,����һ���߳̽�g_bOver���ó�true,ͬʱ���ʱ��cpuִ���л���������
		//���߳���ǡ�����ѵ���Ʒ��������������Ʒ��,���ʱ��ͻᵼ���������߳��˳�,���������̻߳����������
		//ȥ,���ҷŵ�����������Ʒ��û���������߳��ٴ�����,�������ֳ���Ҳûɶ̫�������,�߼��Ϸ����û�����
		//������ֹ���߼��趨,�����û�����������ֹ����˼�����ڻ�û�д�������������Ƶ�ļ�����ֹ

		//�����ݿ��ж�ȡ��������ļ�,�����ȡ����δ�������Ƶ�ļ��ļ�¼
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

				//�Ӳ�ѯ��ÿ�м�¼�ĵڶ����еõ� ScanDirectoryID���������еõ�FileName
				scanDirectoryID = utf8_to_gbk(oneRow.at(1));
				fileName = utf8_to_gbk(oneRow.at(2));
				idPrimaryKey = utf8_to_gbk(oneRow.at(0));
				//����scanDirectoryID��ScanDirectory���ж�ȡ��Ŀ¼��
				//������Ŀ¼��FileName��ϳ�һ�������ľ���·��
				string sqlQueryDirectory = string("select * from ScanDirectory where ID = ")
					+ oneRow.at(1);
				//cout << "sqlQueryDirectory: " << sqlQueryDirectory << endl;
				pdb->m_SDtable.query(resultsDirectory, sqlQueryDirectory);
				if (resultsDirectory.empty()) {
					cout << sqlQueryDirectory << "result is empty" << endl;
					break;
				}
				//���������ѯ��¼ֻ��1��
				assert(resultsDirectory.size() == 1);
				oneRowDirectory = resultsDirectory.at(0);
				//ScanDirectory��ĵ�������path
				string absoluteFilePath = oneRowDirectory.at(2) + '\\' + fileName;
				//����������·���ļ����ŵ�һ��vector<string>��
				vecProduceItem.push_back(absoluteFilePath);
				
				resultsDirectory.clear();
				vector<vector<string>>(resultsDirectory).swap(resultsDirectory);
				oneRowDirectory.clear();
				vector<string>(oneRowDirectory).swap(oneRowDirectory);
				
				v++;
			}
			initialID = atoi(idPrimaryKey.c_str()) + 1;
		}
		//�Ƚ��Ѿ����ɵ���Ʒ������1,Ȼ���ٷŵ�������,�����ȡ���ļ�¼��С��10��,������g_bOverΪtrue
		//�������߳�������ֹ
		gItemNumProduced++;
		gConsumerProducer_videoReadSubmit2ThreadPool.ProduceItem(vecProduceItem);

		//������ݿ��ж�ȡ�ļ�¼����С��10����������g_bOverΪtrue
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
	//������ַ�����utf8��ʽ,FolderUtil��videoUtil�еĺ����ַ���·��ȫ��gbk��,�����漰���ַ�����ʽת��
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
		//ȥ��·����ĩβ��б��
		if (filePath[strlen(filePath) - 1] == '\\')
			filePath[strlen(filePath) - 1] = '\0';
		//�ȴ�ScanDirectory���и���filePath�ҵ�ID,sqlite3���ݿ��д洢���ַ�����ʽ��utf-8
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
		//ScanDirectory��ĵ�һ����ID
		string ScanDirectoryID = oneRowDirectory.at(0);
		//����ScanDirectoryID��fileName��ScanFile�����ҵ���Ӧ�ļ�¼��ID
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

		//���������Ƶ���д��sqlite���ݿ�
		FrameDiffDetect *pFd = new FrameDiffDetect();
		bool bResult = pFd->FrameDetectResult(vImg);
		//cout << "pFd->FrameDetectResult(vImg):" << bResult << endl;
		pdb->m_SFtable.update_bDeleteMark(atoi(id.c_str()), bResult);
		gVideoNumAlreadyHandled++;
		//�ͷ�vector���ڴ�ռ䣬��ֹ�ڴ�й©
		resultsDirectory.clear();
		vector<vector<string>>(resultsDirectory).swap(resultsDirectory);
		oneRowDirectory.clear();
		vector<string>(oneRowDirectory).swap(oneRowDirectory);
		resultsFile.clear();
		vector<vector<string>>(resultsFile).swap(resultsFile);
		oneRowFile.clear();
		vector<string>(oneRowFile).swap(oneRowFile);
		//�ͷ�vector������ڴ�ռ䣬��ֹ�ڴ�й©
		vImg.clear();
		vector<Mat>(vImg).swap(vImg);
		v++;
	}
	
	

	//�ͷ��ڴ�
	vecVideoAbsolutePath.clear();
	vector<string>(vecVideoAbsolutePath).swap(vecVideoAbsolutePath);
	if (pdb != NULL)
		delete pdb;
}

void ConsumerTask_SubmitVideoFile2ThreadPool() // ����������
{
	thread_pool tpVideoProcess(-1);
	//�������ڻ������ǿ����̳߳�thread_pool�е�task�������С�ڵ���5������£�
	//�ӻ�������ȡ�����������Ƶ�ļ�����slotΪ��λ��ÿ��10���ļ�����
	//ִ��thread_pool���submit�����ύ��Ƶ��������
	while (true) { 
		if (g_bOver && (gItemNumConsumed == gItemNumProduced))
			break;

		while ((gItemNumConsumed < gItemNumProduced) && tpVideoProcess.workQueueSize() <= 5) {
			vector<string> vecConsumedItem = gConsumerProducer_videoReadSubmit2ThreadPool.ConsumeItem(); // ����һ����Ʒ.
			
			//vector<string>::iterator v = vecConsumedItem.begin();
			//cout << "vector<string> vecConsumedItem" << endl;
			//while (v != vecConsumedItem.end()) {
			//	cout << *v << endl;
			//	v++;
			//}

			gItemNumConsumed++;
			//�ύthread_pool
			//std::cout << "queueSize:" << tpVideoProcess.workQueueSize() << "tp.submit ...\n";
			tpVideoProcess.submit(std::bind(videoProceed, vecConsumedItem));
		}
		//����2�룬�ȴ�thread_pool������Ƶ
		std::this_thread::sleep_for(std::chrono::seconds(15));
	}

	//�������߳�ÿ��5s��ʱ�ж�threadPool�̳߳ص���������Ƿ�Ϊ�գ�
	//���Ϊ�գ��ڵȴ�60s֮�󣨸�д�����ݿ���Ƶ���������߳�Ԥ��д��Ļ��ᣩ��������������

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
	vecVideoAbsolutePath.push_back(gbk_to_utf8("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\42.mp4"));
	vecVideoAbsolutePath.push_back(gbk_to_utf8("E:\\��������Ƶ����\\����ǽ��\\2018-01-16\\08\\43.mp4"));
	videoProceed(vecVideoAbsolutePath);
}

void printVideoProceedProgress() {
	cout << "��������Ƶ����: " << gVideoNumToHandled << " �Ѵ������: " \
		<< gVideoNumAlreadyHandled << endl;
}

//�����ź� CTRL+C
void sighandler(int signum)
{
	g_bOver = true;
	cout << "�û��жϳ���g_bOver is set to true" << endl;
}

void homeCameraExtractionMainLoop() {
	//ע���жϴ�����
	signal(SIGINT, sighandler);

	InitVideoFileDatabase();
	Timer t;
	//ÿ60s��ӡһ�ν�չ
	t.StartTimer(60000, std::bind(printVideoProceedProgress));
	std::thread producer(ProducerTask_ReadVideoFromDB); // �����������߳�.
	std::thread consumer(ConsumerTask_SubmitVideoFile2ThreadPool); // ��������֮�߳�.
	producer.join();
	consumer.join();
	std::cout << "try to expire timer!" << std::endl;
	t.Expire();

} 