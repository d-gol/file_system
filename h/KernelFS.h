#pragma once

#include "PartitionHandler.h"

#include <Windows.h>
#include <list>
using namespace std;

typedef list<char*> FileOpenList;

struct BlockedThread {
	char fname[20];
	HANDLE blocker;
};

typedef list<BlockedThread> BlockingQueue;

class KernelFS
{
public:
	KernelFS();
	~KernelFS();

	static char mount(Partition* partition);
	static char unmount(char part);
	static char format(char part);
	static char doesExist(char* fname);
	static File* open(char* fname, char mode);
	static char deleteFile(char* fname);
	static char createDir(char* dirname);
	static char deleteDir(char* dirname);
	static char readDir(char* dirname, EntryNum n, Entry &e);

	static HANDLE mutex;

private:

	static PartitionHandler* mountedPartitions[26];
	static bool checkPath(char* path, char &letter, char* name, char* ext);
	static PartitionHandler* getPart(char letter);

	static FileOpenList openFiles;
	static BlockingQueue blocked;

	friend class KernelFile;
};

