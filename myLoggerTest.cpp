#include "myLogger.hpp"
#include "test.h"

#include <thread>

using namespace myLogger;
using namespace std;

int test_myLogger() {
	
	ConsoleLogger logger0;
	
	thread t1([&](){
			
			for (int i = 0; i<100; i++) {
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				logger0(Level::Debug)<<"thread1 print msg "<<"\r\n";
			}
		}
	);
	
	thread t2([&](){
			for (int i = 0; i<100; i++) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				logger0(Level::Info)<< "thread2 output"<<"\r\n";
			}
		}
	);
	
	t1.join();
	t2.join();
	
	return 0;
}