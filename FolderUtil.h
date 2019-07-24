#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "rapidxml/rapidxml.hpp"       
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"

#include <string>  

#include <iostream> 
#include <tchar.h>

using namespace std;

typedef void(*traverseFolder_handler)(string &filePath, ofstream &outFile);

class FolderUtil {
public:
	static bool FolderHasFiles(string fileName);

	static bool listFiles(string fileName);

	static void traverseFolder(string folderPath, ofstream &fout, traverseFolder_handler tf_handler);

	static void traverseFolderAndSave2xml(string folderPath, rapidxml::xml_node<>* node, rapidxml::xml_document<> &doc);

	static int mkdirByLevel(const char* path);

	static int getFolderAndFilename(char* fullPath, char* folder, char* fileName);

};

