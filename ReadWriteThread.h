#pragma once

#include <cstdlib>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>


class ReadWriteThread
{
private:
	std::atomic<bool> bNoReader;
	std::atomic<bool> bNoWriter;

	std::condition_variable CR_noReader;
	std::condition_variable CR_noWriter;

	std::mutex mutex4SynReadWrite;
	std::mutex mutex4ReaderCounter;

	int readCounter = 0;

public:
	ReadWriteThread();
	~ReadWriteThread();
	void readProc(const std::function<void()>& funRead);
	void writeProc(const std::function<void()>& funWrite);
};

