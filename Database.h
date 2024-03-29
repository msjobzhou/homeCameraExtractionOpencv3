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
	void createTable(char* sqlStr);
	void closeDB();
	static int sqlite3_prepare_v2_retry(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
	static int sqlite3_step_retry(sqlite3_stmt *pStatement, string funcName);
public:
	static bool bTableCreated;
	Database(const char* filename);
	~Database();

	class InitialDirectory {
	public:
		Database* m_pParentDB;
		void insert(int id, string path);
		void update(int id, string path);

		void update_bHandledMark(int id, bool bHandledMark);
		void query(vector<vector<string> > &results);
		void query(vector<vector<string> > &results, string sqlQuery);
	} m_IDtable;

	class ScanDirectory {
	public:
		Database* m_pParentDB;
		void insert(int id, int InitialDirectoryID, string path);
		void update_bHandledMark(int id, bool bHandledMark);
		void query(vector<vector<string> > &results);
		void query(vector<vector<string> > &results, string sqlQuery);
	} m_SDtable;

	class ScanFile {
	public:
		Database* m_pParentDB;
		void insert(int id, int ScanDirectoryID, string fileName);
		void update_bDeleteMark(int id, bool bDeleteMark);
		void update_bDeleteAlready(int id, bool bDeleteAlready);
		void query(vector<vector<string> > &results, string sqlQuery);
	} m_SFtable;
};