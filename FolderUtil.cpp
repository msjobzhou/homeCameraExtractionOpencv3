#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream> 
#include <string>  
#include <io.h>  
#include <string>
#include<stdio.h>
#include<stdlib.h>
#include<direct.h>
#include <sys/stat.h>
#include <queue>

#include "FolderUtil.h"
using namespace std;

/*�õ����ݽṹ_finddata_t���ļ���Ϣ�ṹ���ָ�롣
struct _finddata_t
{
unsigned attrib;     //�ļ�����
time_t time_create;  //�ļ�����ʱ��
time_t time_access;  //�ļ���һ�η���ʱ��
time_t time_write;   //�ļ���һ���޸�ʱ��
_fsize_t size;  //�ļ��ֽ���
char name[_MAX_FNAME]; //�ļ���
};

�ļ��������޷���������ȡֵΪ��Ӧ�ĺ꣺_A_ARCH(�浵)��_A_SUBDIR(�ļ���)��_A_HIDDEN(����)��_A_SYSTEM(ϵͳ)��_A_NORMAL(����)��_A_RDONLY(ֻ��)�����׿�����ͨ������ṹ�壬���ǿ��Եõ����ڸ��ļ��ĺܶ���Ϣ��������º��������ǿ��Խ��ļ���Ϣ�洢������ṹ���У�

//��FileName��������ƥ�䵱ǰĿ¼��һ���ļ�
_findfirst(_In_ const char * FileName, _Out_ struct _finddata64i32_t * _FindData);
//��FileName��������ƥ�䵱ǰĿ¼��һ���ļ�
_findnext(_In_ intptr_t _FindHandle, _Out_ struct _finddata64i32_t * _FindData);
//�ر�_findfirst���ص��ļ����
_findclose(_In_ intptr_t _FindHandle);*/

/*
�������ã���ѯ��ǰ�ļ����µ��Ƿ�ֱ�Ӻ����ļ�
����Ŀ¼F1�º�����Ŀ¼F11��F12������֮����Ҳ�����������ļ������ļ�����,F11��������a.txt�������ı��ļ�
��������false��ֻ����F1Ŀ¼��ֱ�Ӻ����ļ�ʱ�ŷ���true����������������false
*/
bool FolderUtil::FolderHasFiles(string fileName)
{
	fileName = fileName + "\\*.*";
	_finddata_t fileInfo;
	intptr_t handle = _findfirst(fileName.c_str(), &fileInfo);

	if (handle == -1L)
	{
		cerr << "failed to find files" << endl;
		return false;
	}

	do
	{
		if (fileInfo.attrib & _A_SUBDIR) {
			if ((strcmp(fileInfo.name, ".") != 0) && (strcmp(fileInfo.name, "..") != 0)) {
				//cout << "folder:" << fileInfo.name << endl;
			}
		}
		else {
		   // cout << "file:" <<fileInfo.name << endl;
			return true;
		}
	} while (_findnext(handle, &fileInfo) == 0);
	_findclose(handle);
	return false;
}

/*
�������ã��ж�һ��������·���Ƿ����ļ���
*/
bool FolderUtil::isFolder(const char* path) {
	struct stat s;
	if (stat(path, &s) == 0) {
		if (s.st_mode & S_IFDIR) {
			//cout << "DIR" << endl;
			return true;
		}
		else if (s.st_mode & S_IFREG) {
			//cout << "FILE" << endl;
			return false;
		}
		else {
			cerr << "cannot identify whether it is folder or file" << endl;
		}
	}
	else {
		cerr << "identify folder or file ERR" << endl;
	}
	return false;
}

/*
	�������ã���ѯ��ǰ�ļ����µ��ļ�
	�����Ҫָ���ض���չ������Ҫ���ļ���ĩβ������չ����
	"C:\\Windows\\*.exe"����˼�ǲ�ѯwindowsĿ¼����չ��Ϊexe���ļ�
*/
bool FolderUtil::listFiles(string fileName, listFile_handler lfh)
{
	fileName = fileName + "\\*.*";
	_finddata_t fileInfo;
	intptr_t handle = _findfirst(fileName.c_str(), &fileInfo);

	if (handle == -1L)
	{
		cerr << "failed to find files" << endl;
		return false;
	}

	do
	{
		if (fileInfo.attrib & _A_SUBDIR) {
			if ((strcmp(fileInfo.name, ".") != 0) && (strcmp(fileInfo.name, "..") != 0)) {

			}

		}
		else {
			string fileName = fileInfo.name;
			lfh(fileName);
			//cout << fileInfo.name << endl;
		}
	} while (_findnext(handle, &fileInfo) == 0);
	_findclose(handle);
	return true;
}

/*
�������ã������ļ��У����ҽ����������ļ�����traverseFolder_handler����ص���������
*/
void FolderUtil::traverseFolder(string folderPath, ofstream &fout, traverseFolder_handler tf_handler = NULL)
{
	_finddata_t FileInfo;
	//�������ָ�������ļ��ĸ�ʽ
	string strfind = folderPath + "\\*.*";
	intptr_t Handle = _findfirst(strfind.c_str(), &FileInfo);

	if (Handle == -1L)
	{
		cerr << "can not match the folder path" << endl;
		exit(-1);
	}
	do {
		//�ж��Ƿ�����Ŀ¼
		if (FileInfo.attrib & _A_SUBDIR)
		{
			//���������Ҫ
			if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
			{
				string newPath = folderPath + "\\" + FileInfo.name;
				traverseFolder(newPath, fout, tf_handler);
				//cout << "find new path:"<< newPath << endl;
			}
		}
		else
		{
			if(NULL != tf_handler) {
				string tmpFileName = folderPath + "\\" + FileInfo.name;
				(*tf_handler)(tmpFileName, fout);
			}
			//cout << folderPath << "\\" << FileInfo.name << endl;
		}
	} while (_findnext(Handle, &FileInfo) == 0);

	strfind.clear();
	string(strfind).swap(strfind);

	_findclose(Handle);
	//���������ݹ�ʹ��fout������ļ���������ܹرգ�����ᵼ�������ݹ麯��д�벻��ȥ
	//fout.close();
}

/*
�������ã������ļ��У������������ļ��л����ļ���д�뵽XML�ļ���
*/
void FolderUtil::traverseFolderAndSave2xml(string folderPath, rapidxml::xml_node<>* node, rapidxml::xml_document<> &doc)
{
	_finddata_t FileInfo;
	//�������ָ�������ļ��ĸ�ʽ
	string strfind = folderPath + "\\*.*";
	intptr_t Handle = _findfirst(strfind.c_str(), &FileInfo);

	if (Handle == -1L)
	{
		cerr << "can not match the folder path" << endl;
		exit(-1);
	}
	do {
		//�ж��Ƿ�����Ŀ¼
		if (FileInfo.attrib & _A_SUBDIR)
		{
			//���������Ҫ
			if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
			{
				string newPath = folderPath + "\\" + FileInfo.name;
				rapidxml::xml_node<>* folder = doc.allocate_node(rapidxml::node_element, "folder", NULL);
				folder->append_attribute(doc.allocate_attribute("Path", doc.allocate_string(FileInfo.name)));
				node->append_node(folder);
				traverseFolderAndSave2xml(newPath, folder, doc);
				cout << "find new path:" << newPath << endl;
			}
		}
		else
		{
			string tmpFileName = folderPath + "\\" + FileInfo.name;
			rapidxml::xml_node<>* fileNode = doc.allocate_node(rapidxml::node_element, "file", doc.allocate_string(FileInfo.name));
			node->append_node(fileNode);
			cout << folderPath << "\\" << FileInfo.name << endl;
		}
	} while (_findnext(Handle, &FileInfo) == 0);

	strfind.clear();
	string(strfind).swap(strfind);

	_findclose(Handle);
	//���������ݹ�ʹ��fout������ļ���������ܹرգ�����ᵼ�������ݹ麯��д�벻��ȥ
	//fout.close();
}

/*
�������ã����ö��в���BFS������ȵķ�ʽ�����ļ��У������������ļ��л����ļ������ص�����tfh����
*/

void FolderUtil::traverseFolderBFS(string path, traverseFolder_handler2 tfh) {
	if (!isFolder(path.c_str())){
		cerr << "not a folder" << endl;
		return;
	}
	queue<string> folderQueue;
	folderQueue.push(path);
	while (!folderQueue.empty()) {
		string folder = folderQueue.front();
		_finddata_t FileInfo;
		//�������ָ�������ļ��ĸ�ʽ
		string strfind = folder + "\\*.*";
		intptr_t Handle = _findfirst(strfind.c_str(), &FileInfo);

		if (Handle == -1L)
		{
			cerr << "can not match the folder path" << endl;
			exit(-1);
		}
		do {
			//�ж��Ƿ�����Ŀ¼
			if (FileInfo.attrib & _A_SUBDIR)
			{
				//���������Ҫ
				if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
				{
					string newPath = folder + "\\" + FileInfo.name;
					folderQueue.push(newPath);
					//�ص�����������·��ֵ�·�����д���
					tfh(newPath);
				}
			}
			else
			{
				string tmpFileName = folder + "\\" + FileInfo.name;
				//�ص�����������·��ֵ��ļ����д���
				tfh(tmpFileName);
			}
		} while (_findnext(Handle, &FileInfo) == 0);

		folderQueue.pop();
	}

}
/*
�������ã������ļ����ľ���·���õ��ļ���·�����ļ���
*/

int FolderUtil::getFolderAndFilename(char* fullPath, char* folder, char* fileName) {

	if (fullPath == NULL || folder == NULL || fileName == NULL) {
		return -1;
	}
	size_t nLen = strlen(fullPath);
	if (nLen >= 255) {
		printf("·��̫��������255");
		return -1;
	}
	char chPathTmp[256];
	strcpy(chPathTmp, fullPath);
	if (chPathTmp[nLen - 1] == '\\') {
		//������һ���ַ���\��ֱ����ʾ�����������Ǹ��ļ���·��
		return -1;
	}
	for (size_t i = nLen - 1; i >= 0; i--) {
		if (chPathTmp[i] == '\\') {
			//�Ӿ���·�������ҵ�һ��\������֮ǰ�Ķ����ļ���·��������֮������ļ���
			strcpy(fileName, &chPathTmp[i + 1]);
			chPathTmp[i + 1] = '\0';
			strcpy(folder, chPathTmp);
			break;
		}
	}
	return 0;
}

/*
	�������ã����Ŀ¼�����ڣ�����㴴��
	���ӣ���C��\f1\f2\f3\ ֻ��C��\���Ŀ¼�Ĵ��ڣ���˴�����㴴��f1��f2,f3�����ļ���
	path������Ҫ��\\��β
*/
int FolderUtil::mkdirByLevel(const char* path) {
	char *tag;
	char bufPath[256];
	size_t nLen = strlen(path);
	if (nLen >=255) {
		printf("�ļ�·�����ڵ���255");
		return -1;
	}
	strcpy(bufPath, path);
	//���bufPath�Ľ�β����\������һ����������һ���ֽڲ��ᵼ��Խ�磬ǰ�涨���bufPath������256���ж�·��
	//��󳤶���255, Ԥ����һ���ֽ�
	if (bufPath[nLen - 1] != '\\') {
		bufPath[nLen] = '\\';
		bufPath[nLen+1] = '\0';
	}
	int nPos = 0;
	for (tag = bufPath; *tag; tag++, nPos++) {
		if (*tag == '\\') {
			//�ҵ�һ��\λ�ã�Ŀ¼��������ǰλ������Ϊ\0��ȡ��·����ʼ�����ļ��е�����·��bufPath
			char chTmp = bufPath[nPos]; //unicode������������������
			bufPath[nPos] = '\0';
			////����ļ��в����ڣ��ʹ���һ��
			if (_access(bufPath, 0) == -1) {
				if (-1 == _mkdir(bufPath)) {
					printf("�����ļ���ʧ��%s", bufPath);
				}
			}
			bufPath[nPos] = chTmp;
		}
	}
	return 0;
}