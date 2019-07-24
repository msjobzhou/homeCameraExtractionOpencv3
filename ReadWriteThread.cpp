#include "ReadWriteThread.h"


ReadWriteThread::ReadWriteThread()
{
	bNoReader = true;
	bNoWriter = true;
}


ReadWriteThread::~ReadWriteThread()
{
}

void ReadWriteThread::readProc(const std::function<void()>& funRead) {
	try {
		std::unique_lock<std::mutex> lck1(mutex4SynReadWrite);
		std::unique_lock<std::mutex> readCounterLck(mutex4ReaderCounter, std::defer_lock);
		while (!bNoWriter) {
			CR_noWriter.wait(lck1);
		}
		readCounterLck.lock();
		readCounter++;
		if (readCounter == 1) {
			bNoReader = false;
		}
		readCounterLck.unlock();
		lck1.unlock();

		funRead();

		readCounterLck.lock();
		readCounter--;
		if (readCounter == 0) {
			CR_noReader.notify_all();
			bNoReader = true;
		}
		readCounterLck.unlock();
	}
	catch (std::exception& e)
	{
		//其他的错误
		std::cout << e.what() << std::endl;
	}
}

void ReadWriteThread::writeProc(const std::function<void()>& funWrite) {
	std::unique_lock<std::mutex> lck1(mutex4SynReadWrite);
	bNoWriter = false;
	while (!bNoReader) {
		CR_noReader.wait(lck1);
	}

	funWrite();

	bNoWriter = true;
	CR_noWriter.notify_all();
	lck1.unlock();
}
void fnRead() {
	std::cout << "reading begin" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::cout << "reading ends" << std::endl;
}

void fnWrite() {
	std::cout << "writing begin" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(3));
	std::cout << "writing ends" << std::endl;
}
//下面这个函数给出了怎么使用ReadWriteThread读写锁的示例
int testReadWriteThread()
{
	ReadWriteThread rwThread;
	std::function<void(void)> readTask = std::bind(&ReadWriteThread::readProc, &rwThread, fnRead);
	std::function<void(void)> writeTask = std::bind(&ReadWriteThread::writeProc, &rwThread, fnWrite);
	std::thread writer(writeTask);
	std::thread reader(readTask);
	std::thread reader2(readTask);
	std::thread reader3(readTask);
	
	reader.join();
	reader2.join();
	reader3.join();
	writer.join();
	return 0;
}