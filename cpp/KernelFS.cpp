#include "File.h"
#include "KernelFS.h"
#include "KernelFile.h"
#include "Utilities.h"

PartitionHandler* KernelFS::mountedPartitions[26];
HANDLE KernelFS::mutex = CreateSemaphore(NULL, 1, 100, NULL);;
FileOpenList KernelFS::openFiles;
BlockingQueue KernelFS::blocked;
KernelFS::KernelFS(){
	//mutex = CreateSemaphore(NULL, 1, 100, NULL);
	//openFiles.clear();
}

KernelFS::~KernelFS(){
	CloseHandle(mutex);
}

char KernelFS::mount(Partition* partition){
	int slot = -1;

	for (int i = 0; i < 26; i++){
		if (mountedPartitions[i] && mountedPartitions[i]->myPart == partition)//ako je vec montirana vrati slovo koje ima
		{
			return indextoLetter(i);
		}
		else if (slot < 0 && 0 == mountedPartitions[i]){
			slot = i;
		}
	}

	if (slot >= 0){//ako je slot i dalje -1 znaci da nema slobodnih mesta za montiranje particije 
		mountedPartitions[slot] = new PartitionHandler(partition);//pravimo PartitionHandler, samim tim i FatHandler za tu particiju
		return indextoLetter(slot);//vratimo slovo koje dobije particija
	}
	else
		return 0;//ili nulu :)
}

char KernelFS::unmount(char part){

	unsigned short index = lettertoIndex(part);

	if (index < 0 || index >= 26 || mountedPartitions[index] == 0)
		return 0;

	mountedPartitions[index]->fatHanlder->flush();

	delete mountedPartitions[index];
	mountedPartitions[index] = 0;

	return 1;
}
char KernelFS::format(char part){
	unsigned index = lettertoIndex(part);

	if (index < 0 || index >= 26 || !mountedPartitions[index])
		return 0;

	mountedPartitions[index]->format();

	return 1;
}

char KernelFS::doesExist(char* fname){
	char letter, name[FNAMELEN + 1], ext[FEXTLEN + 1];
	if (checkPath(fname, letter, name, ext)){
		PartitionHandler *p = getPart(letter);
		if (p){

			EntryHandler ent(name, ext, 0, 0);
			return p->findEntry(ent);
		}
		else
			return 0;
	}
	else
		return 0;
}
File* KernelFS::open(char* fname, char mode){
	char letter, name[FNAMELEN + 1], ext[FEXTLEN + 1];
	if (checkPath(fname, letter, name, ext)){
		PartitionHandler *p = getPart(letter);
		if (p){
			EntryHandler ent(name, ext, 0, 0);

			WaitForSingleObject(mutex, INFINITE);

			bool isOpen = false;
			for (FileOpenList::iterator i = openFiles.begin();
				i != openFiles.end(); i++)
			{
				if (strcmp(*i, fname) == 0){
					isOpen = true;
					break;
				}
			}

			if (isOpen){
				ReleaseSemaphore(mutex, 1, NULL);

				blocked.push_back(BlockedThread());
				strcpy(blocked.back().fname, fname);
				blocked.back().blocker = CreateSemaphore(NULL, 0, 1, NULL);
				WaitForSingleObject(blocked.back().blocker, INFINITE);

				WaitForSingleObject(mutex, INFINITE);//kao sleep
			}

			openFiles.push_back(fname); //dodajem otvoreni fajl

			//da li se fajl tek stvara?
			if (!p->findEntry(ent)){
				p->makeNewEntry(ent);
			}

			KernelFile* kf;
			switch (mode)
			{
			case 'r':
				kf = KernelFile::createR(fname, ent, p);
				break;
			case 'w':
				kf = KernelFile::createW(fname, ent, p);
				break;
			case 'a':
				kf = KernelFile::createA(fname, ent, p);
				break;
			default:
				break;
			}

			File* f = new File();
			f->myImpl = kf;

			ReleaseSemaphore(mutex, 1, NULL);
			return f;
		}
		else
			return 0;
	}
	else
		return 0;

}

char KernelFS::deleteFile(char* fname){
	char letter, name[FNAMELEN + 1], ext[FEXTLEN + 1];
	if (checkPath(fname, letter, name, ext)){
		PartitionHandler *p = getPart(letter);
		if (p){

			EntryHandler ent(name, ext, 0, 0);
			WaitForSingleObject(mutex, INFINITE);

			for (FileOpenList::iterator i = openFiles.begin();
				i != openFiles.end(); i++)
			{
				if (strcmp(*i, fname) == 0){
					ReleaseSemaphore(mutex, 1, NULL);
					return 0;
				}
			}

			if (p->findEntry(ent)){
				KernelFile::freeFile(ent, p);
				p->removeEntry(ent);

				ReleaseSemaphore(mutex, 1, NULL);
				return 1;
			}
			else{
				ReleaseSemaphore(mutex, 1, NULL);
				return 0;
			}

		}
		else
			return 0;
	}
	else
		return 0;
}

char KernelFS::createDir(char* dirname){
	return 0;
}

char KernelFS::deleteDir(char* dirname){
	return 0;
}

char KernelFS::readDir(char* dirname, EntryNum n, Entry &e){
	char letter = dirname[0], name[FNAMELEN + 1], ext[FEXTLEN + 1];
	if (letter >= 'A' && letter <= 'Z'){
		PartitionHandler *p = getPart(letter);
		if (p){
			EntryHandler eh;
			char ret = p->findEntry(n, eh);
			e = eh;
			return ret;
		}
		else
			return 0;
	}
	else
		return 0;
}

bool KernelFS::checkPath(char* path, char& letter, char* name, char* ext){

	memset(name, 0, FNAMELEN + 1);
	memset(ext, 0, FEXTLEN + 1);

	if (path[0] < 'A' && path[0] > 'Z')
		return false;

	letter = path[0];

	if (path[1] != ':')
		return false;

	if (path[2] != '\\')
		return false;

	unsigned int i, j;
	for (i = 0, j = 3; i < FNAMELEN; i++, j++){
		if (path[j] == '.' || path[j] == '\0')
			break;
		name[i] = path[j];
	}

	if (path[j] != '.')
		return false;
	else
		j++;

	for (i = 0; i < FEXTLEN; i++, j++){
		if (path[j] == '\0')
			break;
		ext[i] = path[j];
	}

	return true;
}

PartitionHandler* KernelFS::getPart(char letter){

	return mountedPartitions[letter - 'A'];
}