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

	//homeCameraExtractionMainLoop();

	cout << "串行读120次文件耗时"<< calculateRunTime(testReadVideoSerial) << endl;

	cout << "并行读120次文件耗时" << calculateRunTime(testReadVideo4ThreadsPrallel) << endl;

	return 0;
}




