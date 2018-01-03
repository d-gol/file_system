#pragma once
#include "fs.h"

class EntryHandler:public Entry{

public:

	EntryHandler();
	EntryHandler(char* _name, char* _ext, unsigned long _firstCluster, unsigned long _size);
	~EntryHandler();

	bool operator==(EntryHandler& e) ;
	bool operator==(EntryHandler* e) ;
	bool operator!=(EntryHandler& e) ;
	bool operator!=(EntryHandler* e);

	EntryHandler& operator=(EntryHandler& e);
	EntryHandler& operator=(EntryHandler* e);

	static void copyString(char* dest, char* source, unsigned n);
	static bool equalStr(char* a, char* b, unsigned long max);

};