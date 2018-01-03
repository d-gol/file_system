#include "EntryHandler.h"
#include <cstring>

EntryHandler::EntryHandler(){
	memset(this, 0, sizeof(EntryHandler));
}

EntryHandler::EntryHandler(char* _name, char* _ext, unsigned long _firstCluster, unsigned long _size){
	memset(name, 0, FNAMELEN);
	memset(ext, 0, FEXTLEN);
	copyString(name, _name, FNAMELEN);
	copyString(ext, _ext, FEXTLEN);
	firstCluster = _firstCluster;
	size = _size;
}

EntryHandler::~EntryHandler(){
	
}

bool EntryHandler::operator==(EntryHandler& e) {
	return (equalStr(name, e.name, FNAMELEN) && equalStr(ext, e.ext, FEXTLEN));
}

bool EntryHandler::operator==(EntryHandler* e){
	if (e == 0){
		return (name[0] == '\0');
	}
	else{
		return (equalStr(name, e->name, FNAMELEN) && equalStr(ext, e->ext, FEXTLEN));
	}
}

bool EntryHandler::operator!=(EntryHandler& e) {
	return !(this==&e);
}

bool EntryHandler::operator!=(EntryHandler* e){
	if (e == 0){
		return (name[0] != '\0');
	}
	else{
		return !(equalStr(name, e->name, FNAMELEN) && equalStr(ext, e->ext, FEXTLEN));
	}
}

EntryHandler& EntryHandler::operator=(EntryHandler& e){
	copyString(name, e.name, FNAMELEN);
	copyString(ext, e.ext, FEXTLEN);
	attributes = e.attributes;
	firstCluster = e.firstCluster;
	size = e.size;

	return *this;
}

EntryHandler& EntryHandler::operator=(EntryHandler* e){
	if (e == 0){
		memset(this, 0, sizeof(EntryHandler));
	}
	else{
		copyString(name, e->name, FNAMELEN);
		copyString(ext, e->ext, FEXTLEN);
		attributes = e->attributes;
		firstCluster = e->firstCluster;
		size = e->size;
	}

	return *this;
}


void EntryHandler::copyString(char* dest, char* source, unsigned n){

	if (source != 0){

		unsigned i = 0;

		for (i = 0; i < n; i++){
			if (source[i] == '\0') break;
			dest[i] = source[i];
		}
	}

}

bool EntryHandler::equalStr(char* a, char* b, unsigned long max){

	if (a != 0 && b != 0){

		unsigned i = 0;

		while (1){
			if (i > max) 
				return true;
			if ((a[i] == '\0' && b[i] != '\0') || (a[i] != '\0' && b[i] == '\0') || a[i] != b[i])
				return false;
			if (a[i] == '\0' && b[i] == '\0')
				return true;
			i++;
		}

		return true;
	}

	return false;
}