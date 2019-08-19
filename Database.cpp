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
	m_SFtable.m_pParentDB = this;
	char* sqlStrCreateTableInitialDirectory = "CREATE TABLE IF NOT EXISTS \
		InitialDirectory (ID INTEGER PRIMARY KEY AUTOINCREMENT, Path varchar(255) NOT NULL UNIQUE, \
		HandledMark BOOLEAN DEFAULT NULL)";
	createTable(sqlStrCreateTableInitialDirectory);
	char* sqlStrCreateTableScanDirectory = "CREATE TABLE IF NOT EXISTS \
		ScanDirectory (ID INTEGER PRIMARY KEY AUTOINCREMENT, InitialDirectoryID INTEGER, \
		Path varchar(255) NOT NULL UNIQUE, HandledMark BOOLEAN DEFAULT NULL)";
	createTable(sqlStrCreateTableScanDirectory);
	char* sqlStrCreateTableScanFile = "CREATE TABLE IF NOT EXISTS \
		ScanFile (ID INTEGER PRIMARY KEY AUTOINCREMENT, ScanDirectoryID INTEGER, \
		FileName varchar(255) NOT NULL, DeleteMark BOOLEAN DEFAULT NULL, DeleteAlready BOOLEAN DEFAULT NULL)";
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

	char* query = "INSERT INTO InitialDirectory VALUES(@_id, @_path, false);";

	if (NULL == id) {
		query = "INSERT INTO InitialDirectory VALUES(NULL, @_path, false);";
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

void Database::InitialDirectory::update_bHandledMark(int id, bool bHandledMark)
{
	sqlite3_stmt *pStatement;
	char* query = "UPDATE InitialDirectory SET HandledMark=@_HandledMark WHERE ID=@_id;";

	int rc = sqlite3_prepare_v2(m_pParentDB->m_pdb, query, -1, &pStatement, 0);
	if (rc != SQLITE_OK) exit(-1);

	int idx = -1;
	idx = sqlite3_bind_parameter_index(pStatement, "@_id");
	sqlite3_bind_int(pStatement, idx, id);
	idx = sqlite3_bind_parameter_index(pStatement, "@_HandledMark");
	sqlite3_bind_int(pStatement, idx, bHandledMark);

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
					if (NULL == sqlite3_column_text(pStatement, col)) {
						values.push_back("NULL");
					}
					else {
						values.push_back((char*)sqlite3_column_text(pStatement, col));
					}
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

void Database::InitialDirectory::query(vector<vector<string> > &results, string sqlQuery)
{
	sqlite3_stmt *pStatement;
	const char* query = sqlQuery.c_str();
	//cout << "sqlQuery: " << sqlQuery << endl;
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
					if (NULL == sqlite3_column_text(pStatement, col)) {
						values.push_back("NULL");
					}
					else {
						values.push_back((char*)sqlite3_column_text(pStatement, col));
					}
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

	char* query = "INSERT INTO ScanDirectory VALUES(@_id, @_InitialDirectoryID, @_path, false);";

	if (NULL == id) {
		query = "INSERT INTO ScanDirectory VALUES(NULL, @_InitialDirectoryID, @_path, false);";
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

void Database::ScanDirectory::update_bHandledMark(int id, bool bHandledMark)
{
	sqlite3_stmt *pStatement;
	char* query = "UPDATE ScanDirectory SET HandledMark=@_HandledMark WHERE ID=@_id;";

	int rc = sqlite3_prepare_v2(m_pParentDB->m_pdb, query, -1, &pStatement, 0);
	if (rc != SQLITE_OK) exit(-1);

	int idx = -1;
	idx = sqlite3_bind_parameter_index(pStatement, "@_id");
	sqlite3_bind_int(pStatement, idx, id);
	idx = sqlite3_bind_parameter_index(pStatement, "@_HandledMark");
	sqlite3_bind_int(pStatement, idx, bHandledMark);

	sqlite3_step(pStatement);

	sqlite3_finalize(pStatement);

	string error = sqlite3_errmsg(m_pParentDB->m_pdb);
	if (error != "not an error") cout << query << " " << error << endl;
}


void Database::ScanDirectory::query(vector<vector<string> > &results)
{
	sqlite3_stmt *pStatement;
	char* query = "SELECT * FROM ScanDirectory;";
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
					if (NULL == sqlite3_column_text(pStatement, col)) {
						values.push_back("NULL");
					}
					else {
						values.push_back((char*)sqlite3_column_text(pStatement, col));
					}
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

void Database::ScanDirectory::query(vector<vector<string> > &results, string sqlQuery)
{
	sqlite3_stmt *pStatement;
	const char* query = sqlQuery.c_str();
	//cout << "sqlQuery: " << sqlQuery << endl;
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
					if (NULL == sqlite3_column_text(pStatement, col)) {
						values.push_back("NULL");
					}
					else {
						values.push_back((char*)sqlite3_column_text(pStatement, col));
					}
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

void Database::ScanFile::insert(int id, int ScanDirectoryID, string fileName)
{
	sqlite3_stmt *pStatement;

	char* query = "INSERT INTO ScanFile VALUES(@_id, @_ScanDirectoryID, @_fileName, NULL, NULL);";

	if (NULL == id) {
		query = "INSERT INTO ScanFile VALUES(NULL, @_ScanDirectoryID, @_fileName, NULL, NULL);";
	}

	int rc = sqlite3_prepare_v2(m_pParentDB->m_pdb, query, -1, &pStatement, 0);
	if (rc != SQLITE_OK){
		string errmsg = sqlite3_errmsg(m_pParentDB->m_pdb);
		cerr << "sqlite3_prepare_v2 error:" << query << " " << errmsg << endl;
		exit(-1);
	}

	int idx = -1;

	if (NULL != id) {
		idx = sqlite3_bind_parameter_index(pStatement, "@_id");
		sqlite3_bind_int(pStatement, idx, id);
	}

	idx = sqlite3_bind_parameter_index(pStatement, "@_ScanDirectoryID");
	sqlite3_bind_int(pStatement, idx, ScanDirectoryID);

	idx = sqlite3_bind_parameter_index(pStatement, "@_fileName");
	sqlite3_bind_text(pStatement, idx, fileName.c_str(), -1, SQLITE_STATIC);

	sqlite3_step(pStatement);

	sqlite3_finalize(pStatement);

	string error = sqlite3_errmsg(m_pParentDB->m_pdb);
	if (error != "not an error") cout << query << " " << error << endl;
}

void Database::ScanFile::update_bDeleteMark(int id, bool bDeleteMark) {
	sqlite3_stmt *pStatement;
	char* query = "UPDATE ScanFile SET DeleteMark=@_DeleteMark WHERE ID=@_id;";

	int rc = sqlite3_prepare_v2(m_pParentDB->m_pdb, query, -1, &pStatement, 0);
	if (rc != SQLITE_OK) exit(-1);

	int idx = -1;
	idx = sqlite3_bind_parameter_index(pStatement, "@_id");
	sqlite3_bind_int(pStatement, idx, id);
	idx = sqlite3_bind_parameter_index(pStatement, "@_DeleteMark");
	sqlite3_bind_int(pStatement, idx, bDeleteMark);

	sqlite3_step(pStatement);

	sqlite3_finalize(pStatement);

	string error = sqlite3_errmsg(m_pParentDB->m_pdb);
	if (error != "not an error") cout << query << " " << error << endl;

}

void Database::ScanFile::update_bDeleteAlready(int id, bool bDeleteAlready) {
	sqlite3_stmt *pStatement;
	char* query = "UPDATE ScanFile SET DeleteMark=@_bDeleteAlready WHERE ID=@_id;";

	int rc = sqlite3_prepare_v2(m_pParentDB->m_pdb, query, -1, &pStatement, 0);
	if (rc != SQLITE_OK) exit(-1);

	int idx = -1;
	idx = sqlite3_bind_parameter_index(pStatement, "@_id");
	sqlite3_bind_int(pStatement, idx, id);
	idx = sqlite3_bind_parameter_index(pStatement, "@_bDeleteAlready");
	sqlite3_bind_int(pStatement, idx, bDeleteAlready);

	sqlite3_step(pStatement);

	sqlite3_finalize(pStatement);

	string error = sqlite3_errmsg(m_pParentDB->m_pdb);
	if (error != "not an error") cout << query << " " << error << endl;
}

void Database::ScanFile::query(vector<vector<string> > &results, string sqlQuery)
{
	sqlite3_stmt *pStatement;
	const char* query = sqlQuery.c_str();
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
					if (NULL == sqlite3_column_text(pStatement, col)) {
						values.push_back("NULL");
					} 
					else {
						values.push_back((char*)sqlite3_column_text(pStatement, col));
					}
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

void Database::closeDB()
{
	sqlite3_close(m_pdb);
}