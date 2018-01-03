#include "PartitionHandler.h"
#include <cstring>


PartitionHandler::PartitionHandler(Partition* _myPart){
	myPart = _myPart;
	fatHanlder = new FatHandler(myPart);
}


PartitionHandler::~PartitionHandler(){

	delete fatHanlder;

}

void PartitionHandler::format(){
	fatHanlder->format();

	char zero_buffer[ClusterSize];
	memset(zero_buffer, 0, ClusterSize);
	myPart->writeCluster(fatHanlder->rootStartCl, zero_buffer);

}

ClusterNo PartitionHandler::alloc(){ //skidamo prvi iz liste slobodnih i vracamo broj klastera koji smo spremili za alokaciju
	
	if (fatHanlder->fatCache[fatHanlder->freeStartCl] != 0){ //ako pocetak liste slobodnih pokazuje na neki klaster
		ClusterNo ret = fatHanlder->freeStartCl;
		
		fatHanlder->freeStartCl = fatHanlder->fatCache[fatHanlder->freeStartCl]; //novi pocetak liste slobodnih
		fatHanlder->fatCache[ret] = 0; //stari vise ne pokazuje ni na sta

		fatHanlder->flush(); //sibnemo promenu na disk
		return ret; //vrati broj klastera koji je skinut sa liste sobodnih
	}
	return 0;
}

void PartitionHandler::free(ClusterNo toFree){

	fatHanlder->fatCache[toFree] = fatHanlder->freeStartCl;
	fatHanlder->freeStartCl = toFree;

	fatHanlder->flush();
}

bool PartitionHandler::makeNewEntry(EntryHandler &e){

	EntryHandler buffer[103];

	int br = fatHanlder->rootStartCl; //klaster sa pocetka, ovo je brojac koji se menja, prolazi kroz petlju

	while (br != 0){ // dok ne dodjemo do kraja liste sa podacima
		myPart->readCluster(br, (char*)buffer);
		for (unsigned int i = 0; i < 102; i++){ //trazimo slobodno mesto u klasteru
			if (buffer[i] == 0){
				buffer[i] = e;
				fatHanlder->rootEntryNum++;
				myPart->writeCluster(br, (char*)buffer);
				return true;
			}
		}
		br = fatHanlder->fatCache[br];
	}

	ClusterNo newCl = alloc();
	if (newCl == 0)
		return false;

	fatHanlder->fatCache[br] = newCl;
	memset(buffer, 0, ClusterSize);
	buffer[0] = e;
	myPart->writeCluster(newCl, (char*)buffer);

	return true;
}

bool PartitionHandler::findEntry(EntryHandler &e, bool toDelete, bool toChange){

	EntryHandler buffer[103];

	int br = fatHanlder->rootStartCl;

	while (br != 0){
		myPart->readCluster(br, (char*)buffer);
		for (unsigned int i = 0; i < 102; i++){
			if (buffer[i] == e){
				if (toChange){
					buffer[i] = e;
					myPart->writeCluster(br, (char*)buffer);
				}
				else if (toDelete){
					buffer[i] = 0;
					fatHanlder->rootEntryNum--;
					myPart->writeCluster(br, (char*)buffer);
				}
				else{
					e = buffer[i];
				}
				return true;
			}
		}
		br = fatHanlder->fatCache[br];
	}

	return false;
}

char PartitionHandler::findEntry(EntryNum n, EntryHandler& ret){

	int br = fatHanlder->rootStartCl;
	for (unsigned int i = 0; i < (n / 102); i++){
		if (br == 0)
			return 2; //van granica!
		br = fatHanlder->fatCache[br];
	}

	EntryHandler buffer[103];
	myPart->readCluster(br, (char*)buffer);

	ret = buffer[n % 102];
	
	if (ret == 0)
		return 2; //van granica
	else
		return 1;
}