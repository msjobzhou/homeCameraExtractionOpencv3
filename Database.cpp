#include "Database.h"
#include <iostream>
#include <stdlib.h>

Database::Database(const char* filename)
{
	m_pdb = NULL;
	if (!openDB(filename))
		return;
	m_IDtable.m_pParentDB = this;
	m_SDtable.m_pParentDB = this;
	//m_SFtable.m_pParentDB = this;
	char* sqlStrCreateTableInitialDirectory = "CREATE TABLE IF NOT EXISTS \
		InitialDirectory (ID INTEGER PRIMARY KEY AUTOINCREMENT, Path varchar(255) NOT NULL UNIQUE)";
	createTable(sqlStrCreateTableInitialDirectory);
	char* sqlStrCreateTableScanDirectory = "CREATE TABLE IF NOT EXISTS \
		ScanDirectory (ID INTEGER PRIMARY KEY AUTOINCREMENT, InitialDirectoryID INTEGER, \
		Path varchar(255) NOT NULL UNIQUE)";
	createTable(sqlStrCreateTableScanDirectory);
	char* sqlStrCreateTableScanFile = "CREATE TABLE IF NOT EXISTS \
		ScanFile (ID INTEGER PRIMARY KEY AUTOINCREMENT, ScanDirectoryID INTEGER, \
		FileName varchar(255) NOT NULL, DeleteMark BOOLEAN, DeleteAlready BOOLEAN)";
	createTable(sqlStrCreateTableScanFile);
}

Database::~Database()
{
	closeDB();
}

bool Database::openDB(const char* filename)
{
	if (sqlite3_open(filename, &m_pdb) == SQLITE_OK)
		return true;

	return false;
}

void Database::createTable(char* sqlStr)
{
	sqlite3_stmt *pStatement;
	
	int rc = sqlite3_prepare_v2(m_pdb, sqlStr, -1, &pStatement, 0);
	if (rc != SQLITE_OK)
	{
		cerr << "sqlite3_prepare_v2 error:" << sqlStr << endl;
		exit(-1);
	}

	sqlite3_step(pStatement);

	sqlite3_finalize(pStatement);

	string error = sqlite3_errmsg(m_pdb);
	if (error != "not an error") cout << sqlStr << " " << error << endl;
}



void Database::InitialDirectory::insert(int id, string path)
{
	sqlite3_stmt *pStatement;

	char* query = "INSERT INTO InitialDirectory VALUES(@_id, @_path);";

	if (NULL == id) {
		query = "INSERT INTO InitialDirectory VALUES(NULL, @_path);";
	}

	int rc = sqlite3_prepare_v2(m_pParentDB->m_pdb, query, -1, &pStatement, 0);
	if (rc != SQLITE_OK){
		cerr << "sqlite3_prepare_v2 error:" << query << endl;
		exit(-1);
	}

	int idx = -1;
	
	if (NULL != id) {
		idx = sqlite3_bind_parameter_index(pStatement, "@_id");
		sqlite3_bind_int(pStatement, idx, id);
	}

	idx = sqlite3_bind_parameter_index(pStatement, "@_path");
	sqlite3_bind_text(pStatement, idx, path.c_str(), -1, SQLITE_STATIC);

	sqlite3_step(pStatement);

	sqlite3_finalize(pStatement);

	string error = sqlite3_errmsg(m_pParentDB->m_pdb);
	if (error != "not an error") cout << query << " " << error << endl;
}

void Database::InitialDirectory::update(int id, string path)
{
	sqlite3_stmt *pStatement;
	char* query = "UPDATE InitialDirectory SET Path=@_path WHERE ID=@_id;";

	int rc = sqlite3_prepare_v2(m_pParentDB->m_pdb, query, -1, &pStatement, 0);
	if (rc != SQLITE_OK) exit(-1);

	int idx = -1;
	idx = sqlite3_bind_parameter_index(pStatement, "@_id");
	sqlite3_bind_int(pStatement, idx, id);
	idx = sqlite3_bind_parameter_index(pStatement, "@_path");
	sqlite3_bind_text(pStatement, idx, path.c_str(), -1, SQLITE_STATIC);

	sqlite3_step(pStatement);

	sqlite3_finalize(pStatement);

	string error = sqlite3_errmsg(m_pParentDB->m_pdb);
	if (error != "not an error") cout << query << " " << error << endl;
}

void Database::InitialDirectory::query(vector<vector<string> > &results)
{
	sqlite3_stmt *pStatement;
	char* query = "SELECT * FROM InitialDirectory;";
	if (sqlite3_prepare_v2(m_pParentDB->m_pdb, query, -1, &pStatement, 0) == SQLITE_OK)
	{
		int cols = sqlite3_column_count(pStatement);
		int result = 0;
		while (true)
		{
			result = sqlite3_step(pStatement);

			if (result == SQLITE_ROW)
			{
				vector<string> values;
				for (int col = 0; col < cols; col++)
				{
					values.push_back((char*)sqlite3_column_text(pStatement, col));
				}
				results.push_back(values);
			}
			else
			{
				break;
			}
		}

		sqlite3_finalize(pStatement);
	}

	string error = sqlite3_errmsg(m_pParentDB->m_pdb);
	if (error != "not an error") cout << query << " " << error << endl;

}

void Database::ScanDirectory::insert(int id, int InitialDirectoryID, string path)
{
	sqlite3_stmt *pStatement;

	char* query = "INSERT INTO ScanDirectory VALUES(@_id, @_InitialDirectoryID, @_path);";

	if (NULL == id) {
		query = "INSERT INTO ScanDirectory VALUES(NULL, @_InitialDirectoryID, @_path);";
	}

	int rc = sqlite3_prepare_v2(m_pParentDB->m_pdb, query, -1, &pStatement, 0);
	if (rc != SQLITE_OK){
		cerr << "sqlite3_prepare_v2 error:" << query << endl;
		exit(-1);
	}

	int idx = -1;

	if (NULL != id) {
		idx = sqlite3_bind_parameter_index(pStatement, "@_id");
		sqlite3_bind_int(pStatement, idx, id);
	}

	idx = sqlite3_bind_parameter_index(pStatement, "@_InitialDirectoryID");
	sqlite3_bind_int(pStatement, idx, InitialDirectoryID);

	idx = sqlite3_bind_parameter_index(pStatement, "@_path");
	sqlite3_bind_text(pStatement, idx, path.c_str(), -1, SQLITE_STATIC);

	sqlite3_step(pStatement);

	sqlite3_finalize(pStatement);

	string error = sqlite3_errmsg(m_pParentDB->m_pdb);
	if (error != "not an error") cout << query << " " << error << endl;
}

void Database::closeDB()
{
	sqlite3_close(m_pdb);
}