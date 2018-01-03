#include "KernelFile.h"
#include "KernelFS.h";
#include <list>
#include <queue>

HANDLE &KernelFile::mutex = KernelFS::mutex;
FileOpenList &KernelFile::openFiles = KernelFS::openFiles;
BlockingQueue &KernelFile::blocked = KernelFS::blocked;

KernelFile::KernelFile(){
	
}

KernelFile::~KernelFile(){
	closeFile();
}

char KernelFile::write(BytesCnt n, char* buffer){
	if (mode == 'w' || mode == 'a'){
		if (entryHandler.firstCluster == 0)
			initFile();

		for (BytesCnt i = 0; i < n; i++){
			this->buffer[cursor % 2048] = buffer[i];
			cursor++;

			if (cursor % 2048 == 0){
				part->myPart->writeCluster(currentCluster, (char*)this->buffer);
				if (!part->fatHanlder->fatCache[currentCluster]){
					//fileList->addBack(part->alloc());
					part->fatHanlder->fatCache[currentCluster] = part->alloc();
					currentCluster = part->fatHanlder->fatCache[currentCluster];
					part->fatHanlder->flush();
					if (currentCluster == 0)
						return 0;
					memset(this->buffer, 0, ClusterSize);
				}
				else{
					currentCluster = part->fatHanlder->fatCache[currentCluster];
					part->myPart->readCluster(currentCluster, (char*)this->buffer);
				}
			}
		}

		if (entryHandler.size < cursor)
			entryHandler.size = cursor;
		
		return 1;
	}
	return 0;
}

BytesCnt KernelFile::read(BytesCnt n, char* buffer){
	if (entryHandler.firstCluster == 0)
		return 0;

	unsigned int i = 0;
	while (cursor < entryHandler.size){
		if (i < n){
			buffer[i] = this->buffer[cursor % 2048];
			cursor++;
			i++;
			if (cursor % 2048 == 0){
				if (!part->fatHanlder->fatCache[currentCluster])
				break;
				currentCluster = part->fatHanlder->fatCache[currentCluster];
				part->myPart->readCluster(currentCluster, (char*)this->buffer);
			}
		}
		else{
			break; 
		}
	}

	return i;
}

char KernelFile::seek(BytesCnt n){ 

	if (n < entryHandler.size){

		part->myPart->writeCluster(currentCluster, (char*)buffer);
		currentCluster = entryHandler.firstCluster;
		///int kl = currentCluster;
		for (unsigned int i = 0; i < (n / 2048); i++){
			currentCluster = part->fatHanlder->fatCache[currentCluster];
		}
		//currentCluster = kl;

		part->myPart->readCluster(currentCluster, (char*)buffer);
		
		cursor = n;

		return 1;
	}
	else
		return 0;
}
BytesCnt KernelFile::filePos(){
	return cursor;
}
char KernelFile::eof(){
	return (entryHandler.size == cursor);
}
BytesCnt KernelFile::getFileSize(){
	return entryHandler.size;
}

KernelFile* KernelFile::createR(char* path, EntryHandler& e, PartitionHandler* p){// poziva se iz open
	KernelFile* ret = new KernelFile();

	strcpy(ret->path, path);

	ret->cursor = 0;
	ret->entryHandler = e;
	ret->currentCluster = e.firstCluster;
	ret->part = p;
	
	ret->part->myPart->readCluster(ret->currentCluster, ret->buffer);
	ret->mode = 'r';

	return ret;
}

KernelFile* KernelFile::createW(char* path, EntryHandler& e, PartitionHandler* p){//entry se prosledjuje iz opena
	KernelFile* ret = new KernelFile();

	strcpy(ret->path, path);

	if (e.firstCluster != 0){
		freeFile(e, p);
	}

	ret->cursor = 0;
	ret->entryHandler = e;
	ret->entryHandler.size = 0;
	ret->entryHandler.firstCluster = 0;
	ret->currentCluster = e.firstCluster;
	ret->part = p;
	
	memset(ret->buffer, 0, ClusterSize);
	ret->mode = 'w';

	return ret;
}

KernelFile* KernelFile::createA(char* path, EntryHandler& e, PartitionHandler* p){
	KernelFile* ret = new KernelFile();

	strcpy(ret->path, path);

	ret->entryHandler = e;
	ret->part = p;

	int i = 0;
	ClusterNo br = e.firstCluster;
	while (i <= e.size / 2048 && p->fatHanlder->fatCache[br] != 0){
		i++;
		br = p->fatHanlder->fatCache[br];
	}
	ret->currentCluster = br;

	ret->cursor = e.size;
	ret->part->myPart->readCluster(ret->currentCluster, ret->buffer);
	ret->mode = 'a';

	return ret;
}

//createw, createa, close (vrati na disk, obrisi fajl listu, zovem u destruktoru)

void KernelFile::freeFile(EntryHandler e, PartitionHandler* p){
	char buf[2048];
	int br = e.firstCluster;
	int i = 0;
	while(i <= (e.size/2048)){//idemo kroz klastere koje zauzima fajl
		memset(buf, 0, ClusterSize);
		int pom = p->fatHanlder->fatCache[br];
		p->myPart->writeCluster(br, buf);//u stvarni klaster sa podacima od fajla upisemo nule
		p->fatHanlder->fatCache[br] = p->fatHanlder->freeStartCl;//izbacujemo redom, tj dodajemo u listu slobodnih
		p->fatHanlder->freeStartCl = br;
		br = pom;
		i++;
	}
	e.size = 0;
	p->fatHanlder->flush();
}

void KernelFile::initFile(){
	ClusterNo newCl = part->alloc();
	entryHandler.firstCluster = newCl;
	part->replaceEntry(entryHandler);
	currentCluster = newCl;
	memset(buffer, 0, ClusterSize);
}

void KernelFile::closeFile(){
	WaitForSingleObject(mutex, INFINITE);

	part->myPart->writeCluster(currentCluster, buffer);
	part->replaceEntry(entryHandler);

	for (BlockingQueue::iterator i = blocked.begin();
		i != blocked.end(); i++)
	{
		if (strcmp(i->fname, path) == 0){
			ReleaseSemaphore(i->blocker, 1, NULL);
			CloseHandle(i->blocker);
			blocked.erase(i);
			break;
		}
	}

	for (FileOpenList::iterator i = openFiles.begin(); 
		i != openFiles.end(); i++)
	{
		if (strcmp(*i, path) == 0){
			openFiles.erase(i);
			break;
		}
	}

	ReleaseSemaphore(mutex, 1, NULL);
}