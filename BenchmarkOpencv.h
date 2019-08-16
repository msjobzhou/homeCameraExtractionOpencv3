#pragma once

typedef void(*VOID_FUNC)();

double calculateRunTime(VOID_FUNC pVoidFunc);

void testCreateFileCapture();

void readVideoEveryXFramesInNthPart(const char* fileFullPath, int NSegments = 1, int nth = 1, int period = 50);

void task1();

void task2();

void testCreateFileCapture2Threads();

void test_read_video_thread_pool();

void test_read_video();

void testReadVideo4ThreadsPrallel();

void testReadVideoSerial();