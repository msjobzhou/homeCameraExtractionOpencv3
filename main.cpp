#include"test.h"

#include "homeCameraExtractionMain.h"
#include "BenchmarkOpencv.h"
#include <iostream>
#include "netClient.h"
#include "netServer.h"
#include <thread>


using namespace std;


int main()
{
	//test_readVideoSaveImg();

	//test_FrameDetectResult();

	//test_FrameDetectResultAndSaveVideo();

	//test_sqlite();

	//test_sqlite_zhongwen();

	//test_FolderHasFiles();

	//test_Database_class();

	//testReadWriteThread();

	//testTimer();

	//test_thread_pool();

	//benchmark_single_thread();

	//benchmark_multiple_thread();

	//test_zhongwen();

	//test_traverseFolderBFS();

	//test_SingleConsumerSingleProducer_class();

	//thread tClient = thread(netClientTestWetherServerAlive);
	//cout << calculateRunTime(homeCameraExtractionMainLoop) << endl;
	//tClient.join();
	//deleteHomeCameraVideoFile();

	//cout << "1���̴߳��ж�120���ļ���ʱ"<< calculateRunTime(testReadVideoSerial) << endl;

	//cout << "4���̲߳��ж�120���ļ���ʱ" << calculateRunTime(testReadVideo4ThreadsPrallel) << endl;

	//cout << calculateRunTime(testCreateFileCapture) << endl;

	//cout << calculateRunTime(testReadVideoSeekPos) << endl;
	
	//cout << calculateRunTime(testReadVideoEveryFrame) << endl;

	//cout << __FUNCTION__ << endl;

	//test_get_current_time();

	//test_myLogger();

	//test_createProcess();

	//test_server_client_communication();

	netServerSelect();

	return 0;
}




