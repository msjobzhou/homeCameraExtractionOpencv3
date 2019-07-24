#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>

using namespace std;

class Database
{
private:
	sqlite3 *m_pdb;

	bool openDB(const char* filename);
	void createTable();
	void closeDB();

public:
	Database(const char* filename);
	~Database();

	class InitialDirectory {
	public:
		Database* m_pParentDB;
		void insert(int id, string path);
		void update(int id, string path);
		void query(vector<vector<string> > &results);
	} m_IDtable;
};