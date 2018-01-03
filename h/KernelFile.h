#pragma once

#include "File.h"
#include "fs.h"
#include "part.h"
#include "PartitionHandler.h"
#include "KernelFS.h"

#include <Windows.h>

class KernelFile
{
public:
	static KernelFile* createR(char*, EntryHandler&, PartitionHandler*);
	static KernelFile* createW(char*, EntryHandler&, PartitionHandler*);
	static KernelFile* createA(char*, EntryHandler&, PartitionHandler*);
	~KernelFile();
	char write(BytesCnt, char* buffer);
	BytesCnt read(BytesCnt, char* buffer);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();

	KernelFile();

	static HANDLE &mutex;

private:

	BytesCnt cursor;
	EntryHandler entryHandler; //tu je i prvi klaster koji fajl zauzima
	char buffer[2048];
	char path[20];
	PartitionHandler* part;
	//FatList* fileList;
	char mode;
	ClusterNo currentCluster;
	//ClusterNo lastCluster; //poslednji klaster koje fajl zauzima

	friend class KernelFS;

	static void freeFile(EntryHandler, PartitionHandler*); //oslobodja fajl
	void initFile();
	void closeFile();

	static FileOpenList &openFiles;
	static BlockingQueue &blocked;
	static BlockingQueue &blockedUnmount;
};