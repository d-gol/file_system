#include "fs.h"
#include "KernelFS.h"


FS::FS(){
	//myImpl = new KernelFS();
}

char FS::mount(Partition* partition){
	return KernelFS::mount(partition);
}

char FS::unmount(char part){
	return KernelFS::unmount(part);
}

char FS::format(char part){
	return KernelFS::format(part);
}

char FS::doesExist(char* fname){
	return KernelFS::doesExist(fname);
}

File* FS::open(char* fname, char mode){
	return KernelFS::open(fname, mode);
}
char FS::deleteFile(char* fname){
	return KernelFS::deleteFile(fname);
}

char FS::createDir(char* dirname){
	return KernelFS::createDir(dirname);
}

char FS::deleteDir(char* dirname){
	return KernelFS::deleteDir(dirname);
}

char FS::readDir(char* dirname, EntryNum n, Entry &e){
	return KernelFS::readDir(dirname,n,e);
}
