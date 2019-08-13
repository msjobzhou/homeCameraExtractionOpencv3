#pragma once
#define _CRT_SECURE_NO_WARNINGS 1

#include <string>
#include <vector>

using namespace std;

void showImg();
void test_readVideoSaveImg();
void test_FrameDetectResult();
void test_FrameDetectResultAndSaveVideo();
void test_camera();
void test_sqlite();
int test_sqlite_zhongwen();
void test_Database_class();
int testReadWriteThread();
void testTimer();
int test_thread_pool();
void benchmark_single_thread();
void benchmark_multiple_thread();
void test_zhongwen();
void test_FolderHasFiles();
void printString(string& s);

void tfh_sqlite(string& s);
void test_traverseFolderBFS();


void ProducerTask();
void ConsumerTask();
void test_SingleConsumerSingleProducer_class();

int test_thread_pool();
void testTimer();