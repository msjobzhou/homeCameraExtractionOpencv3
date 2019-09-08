#include"test.h"

#include "homeCameraExtractionMain.h"
#include "BenchmarkOpencv.h"
#include <iostream>

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

	cout << calculateRunTime(homeCameraExtractionMainLoop) << endl;
	//deleteHomeCameraVideoFile();

	//cout << "1个线程串行读120次文件耗时"<< calculateRunTime(testReadVideoSerial) << endl;

	//cout << "4个线程并行读120次文件耗时" << calculateRunTime(testReadVideo4ThreadsPrallel) << endl;

	//cout << calculateRunTime(testCreateFileCapture) << endl;

	//cout << calculateRunTime(testReadVideoSeekPos) << endl;
	
	//cout << calculateRunTime(testReadVideoEveryFrame) << endl;

	//cout << __FUNCTION__ << endl;

	//test_get_current_time();

	//test_myLogger();

	return 0;
}




