#pragma once

#include <sqlite3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <atomic>
#include <mutex>
#include "Timer.hpp"
#include "thread_pool.hpp"

using namespace std;
char sql[200] = "";
sqlite3 *database;
//����sqlite���߳�ģʽ�£�mutex on database connection and prepared statement objects.
mutex g_mtxConnectionAndPreparedStatement;

//���ڶ�ʱ��ӡ��ÿ��һ��ʱ��Ķ�д���ݿ����
int lastReadCount = 0;
atomic<int> readCount = 0;
int lastWriteCount = 0;
atomic<int> writeCount = 0;

//count�����ŵ���ʱ���У�ÿ��һ��ʱ��ִ��һ��
void count() {
	int lastRead = lastReadCount;
	int lastWrite = lastWriteCount;
	lastReadCount = readCount.load();
	lastWriteCount = writeCount.load();
	cout << "read times: " << lastReadCount - lastRead << "  write times: " << lastWriteCount - lastWrite << endl;
}



sqlite3* open_database() {
	sqlite3 *conn = NULL;
	char *err_msg = NULL;
	
	//�����ݿ�,��������;
	if (sqlite3_open("test_for_cpp.db", &conn) != SQLITE_OK)
	{
		sqlite3_close(conn);
		cout << "Failed to open database: " << sqlite3_errmsg(conn) << endl;
	}
	return conn;
}

void init_test_table() {
	database = open_database();

	//���������õı�
	char *sql = "CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY AUTOINCREMENT, value INTEGER);";
	char *errorMsg = 0;
	int rc = sqlite3_exec(database, sql, NULL, NULL, &errorMsg);
	if (rc != SQLITE_OK){
		cout << "SQL error:  " << errorMsg << endl;
		sqlite3_free(errorMsg);
	}

	//���в���1000����������
	if (sqlite3_exec(database, "BEGIN TRANSACTION", NULL, NULL, &errorMsg) != SQLITE_OK) {
		cout<<"Failed to begin transaction: "<<errorMsg<<endl;
	}

	static const char *insert = "INSERT INTO test VALUES (NULL, ?);";
	sqlite3_stmt *stmt;
	srand(int(time(0)));
	if (sqlite3_prepare_v2(database, insert, -1, &stmt, NULL) == SQLITE_OK) {
		for (int i = 0; i < 1000; ++i) {
			sqlite3_bind_int(stmt, 1, rand());
			if (sqlite3_step(stmt) != SQLITE_DONE) {
				--i;
				cout<<"Error inserting table: "<< sqlite3_errmsg(database)<<endl;
			}
			sqlite3_reset(stmt);
		}
		sqlite3_finalize(stmt);
	}

	if (sqlite3_exec(database, "COMMIT TRANSACTION", NULL, NULL, &errorMsg) != SQLITE_OK) {
		cout << "Failed to commit transaction: "<< errorMsg << endl;
	}

	static const char *query = "SELECT count(*) FROM test;";
	if (sqlite3_prepare_v2(database, query, -1, &stmt, NULL) == SQLITE_OK) {
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			cout << "Table size: "<< sqlite3_column_int(stmt, 0) << endl;
		}
		else {
			cout << "Failed to read table: "<< sqlite3_errmsg(database) << endl;
		}
		sqlite3_finalize(stmt);
	}


	sqlite3_close(database);
}

//�˺���������򿪺͹ر����ݿ�����
void readData() {
	//const char *query = "SELECT value FROM test WHERE value < ? ORDER BY value DESC LIMIT 1;";
	const char *query = "SELECT value FROM test WHERE id = ?;";

	sqlite3_stmt *stmt;
	srand(int(time(0)));
	if (sqlite3_prepare_v2(database, query, -1, &stmt, NULL) == SQLITE_OK) {
		sqlite3_bind_int(stmt, 1, rand() % 1000 + 1);
		int returnCode = sqlite3_step(stmt);
		if (returnCode == SQLITE_ROW || returnCode == SQLITE_DONE) {
			++readCount;
		}
		sqlite3_finalize(stmt);
	}
	else {
		cout << "Failed to prepare statement: " << sqlite3_errmsg(database) << endl;
	}
}


void writeData() {
	const char *update = "UPDATE test SET value = ? WHERE id = ?;";

	sqlite3_stmt *stmt;
	srand(int(time(0)));
	if (sqlite3_prepare_v2(database, update, -1, &stmt, NULL) == SQLITE_OK) {
		sqlite3_bind_int(stmt, 1, rand());
		sqlite3_bind_int(stmt, 2, rand() % 1000 + 1);
		if (sqlite3_step(stmt) == SQLITE_DONE) {
			++writeCount;
		}
		sqlite3_finalize(stmt);
	}
	else {
		cout << "Failed to prepare statement: " << sqlite3_errmsg(database) << endl;
	}
}

void benchmark_single_thread() {
	sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
	//cout << "sqlite3_threadsafe :" << sqlite3_threadsafe() << endl;
	cout << "sqlite3_libversion :" << sqlite3_libversion() << endl;
	database = open_database();
	char *errorMsg = 0;
	//����WALģʽ��д��������δ����WALǰ��7������
	
	if (sqlite3_exec(database, "PRAGMA journal_mode=DELETE;", NULL, NULL, &errorMsg) != SQLITE_OK) {
		cout << "Failed to set WAL mode: " << errorMsg << endl;
		sqlite3_free(errorMsg);
	}

	sqlite3_wal_checkpoint(database, NULL); // ÿ�β���ǰ��checkpoint������WAL�ļ������Ӱ������
	
	//init_test_table();
	//����ֻ��1���̵߳�thread_pool���ύ1��ζ���˳��ִ��
	thread_pool tp(1);
	for (int i = 0; i<100000; i++)
	{
		tp.submit(readData);
	}

	//�Ȳ���ÿ����Զ����ٴΣ�����ʱ��10s

	Timer t;
	//������ִ�ж�ʱ���񣬴�ӡ��ÿ����Զ������д���ٴ�
	t.StartTimer(1000, count);
	std::this_thread::sleep_for(std::chrono::seconds(10));
	std::cout << "try to expire timer!" << std::endl;
	t.Expire();

	sqlite3_close(database);

}

sqlite3 *openDb4thread() {
	sqlite3 *database_tmp = NULL;
	if (sqlite3_open("test_for_cpp.db", &database_tmp) != SQLITE_OK) {
		sqlite3_close(database_tmp);
		cout << "Failed to open database: " << sqlite3_errmsg(database_tmp) << endl;
	}
	return database_tmp;
}

void readData_multiple_thread() {
	unique_lock<std::mutex> sqlLck(g_mtxConnectionAndPreparedStatement);
	sqlite3 *database_read = openDb4thread();
	//const char *query = "SELECT value FROM test WHERE value < ? ORDER BY value DESC LIMIT 1;";
	const char *query = "SELECT value FROM test WHERE id = ?;";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(database_read, query, -1, &stmt, NULL);
	sqlLck.unlock();
	srand(int(time(0)));
	if (rc == SQLITE_OK) {
		sqlite3_bind_int(stmt, 1, rand() % 1000 + 1);
		int returnCode = sqlite3_step(stmt);
		if (returnCode == SQLITE_ROW || returnCode == SQLITE_DONE) {
			++readCount;
		}
		sqlite3_finalize(stmt);
	}
	else {
		cout << "Failed to prepare statement: " << sqlite3_errmsg(database_read) << endl;
	}
	sqlite3_close(database_read);
}


void writeData_multiple_thread() {
	//sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
	unique_lock<std::mutex> sqlLck(g_mtxConnectionAndPreparedStatement);
	sqlite3 *database_write = openDb4thread();
	const char *update = "UPDATE test SET value = ? WHERE id = ?;";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(database_write, update, -1, &stmt, NULL);
	sqlLck.unlock();

	srand(int(time(0)));
	if (rc == SQLITE_OK) {
		sqlite3_bind_int(stmt, 1, rand());
		sqlite3_bind_int(stmt, 2, rand() % 1000 + 1);
		if (sqlite3_step(stmt) == SQLITE_DONE) {
			++writeCount;
		}
		sqlite3_finalize(stmt);
	}
	else {
		cout << "Failed to prepare statement: " << sqlite3_errmsg(database_write) << endl;
	}
	sqlite3_close(database_write);
}

void benchmark_multiple_thread() {
	//sqlite3_shutdown();
	int rc=sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
	//sqlite3_initialize();
	cout << "sqlite3_config_result :" << rc << endl;
	//cout << "sqlite3_threadsafe :" << sqlite3_threadsafe() << endl;
	cout << "sqlite3_libversion :" << sqlite3_libversion() << endl;
	//database = open_database();
	//char *errorMsg = 0;
	//����WALģʽ��д��������δ����WALǰ��7������
	//if (sqlite3_exec(database, "PRAGMA journal_mode=WAL;", NULL, NULL, &errorMsg) != SQLITE_OK) {
	//	cout << "Failed to set WAL mode: " << errorMsg << endl;
	//	sqlite3_free(errorMsg);
	//}

	//sqlite3_wal_checkpoint(database, NULL); // ÿ�β���ǰ��checkpoint������WAL�ļ������Ӱ������

	//init_test_table();
	//����ֻ��4���̵߳�thread_pool���ύ1��ζ���˳��ִ��
	thread_pool tp(4);
	for (int i = 0; i<100000; i++)
	{
		tp.submit(readData_multiple_thread);
	}

	//�Ȳ���ÿ����Զ����ٴΣ�����ʱ��10s

	Timer t;
	//������ִ�ж�ʱ���񣬴�ӡ��ÿ����Զ������д���ٴ�
	t.StartTimer(1000, count);
	std::this_thread::sleep_for(std::chrono::seconds(10));
	std::cout << "try to expire timer!" << std::endl;
	t.Expire();

	//sqlite3_close(database);

}