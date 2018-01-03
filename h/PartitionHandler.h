#pragma once

#include "part.h"
#include "FatHandler.h"
#include "EntryHandler.h"
#include <cstring>

class PartitionHandler
{
public:
	PartitionHandler(Partition* _myPart);
	PartitionHandler() { myPart = 0; }
	~PartitionHandler();

	ClusterNo alloc();
	void free(ClusterNo n);

	void format();

	bool makeNewEntry(EntryHandler& e); //upise novi zapis na particiju

	inline bool findEntry(EntryHandler &e) { return findEntry(e, false, false); }
	inline bool replaceEntry(EntryHandler &e) { return findEntry(e, false, true); }
	inline bool removeEntry(EntryHandler &e) { return findEntry(e, true, false); }
	char findEntry(EntryNum no, EntryHandler &ret);

	friend class KernelFS;
	friend class KernelFile;

private:

	Partition *myPart;
	FatHandler *fatHanlder;
	/*bool mounted;
	bool formated;*/

	bool findEntry(EntryHandler &e, bool toDelete, bool toChange);
};

