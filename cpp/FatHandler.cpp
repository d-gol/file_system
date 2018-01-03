#include "FatHandler.h"
#include "part.h"

#include <cstring>


FatHandler::FatHandler(Partition* _myPart) : myPart(_myPart){
	initFat();
	load();
}


FatHandler::~FatHandler(){
	delete fatCache;
}

void FatHandler::initFat(){
	clustersTaken = 1;
	fatNum = myPart->getNumOfClusters();
	if (fatNum > ZeroClusterFatEntries){
		ClusterNo t = fatNum - ClusterFatEntries;
		clustersTaken += t / ClusterFatEntries;
		if (fatNum % ClusterFatEntries)
			clustersTaken++;
	}

	fatCache = new ClusterNo[fatNum];
}

void FatHandler::format(){
	freeStartCl = clustersTaken + 1;
	rootStartCl = clustersTaken;
	rootEntryNum = 0;

	//podesavam fat cache da bude u inicijalnom stanju
	//prvih N+1 ulaza su 0 zbog FAT tabele i jedan za root
	memset(fatCache, 0, (clustersTaken + 1) * sizeof(ClusterNo));

	for (ClusterNo i = clustersTaken + 1; i < fatNum - 1; i++){
		fatCache[i] = i + 1; //pokazuje na sledeci
	}

	fatCache[fatNum - 1] = 0; //kraj liste slobodnih

	flush(); //ima izmena, ceo na disk
}

void FatHandler::flush(){

	ClusterNo sendBuffer[ClusterFatEntries];
	//Nulti klaster:
	sendBuffer[0] = freeStartCl;
	sendBuffer[1] = rootStartCl;
	sendBuffer[2] = fatNum;
	sendBuffer[3] = rootEntryNum;

	//Svi klasteri se upisuju:
	ClusterNo i, clNo = 0, cnt = 4;
	sendBuffer[cnt++] = fatCache[0];
	for (i = 1; i < fatNum; i++)
	{
		sendBuffer[cnt++] = fatCache[i];
		if (cnt % ClusterFatEntries == 0){
			myPart->writeCluster(clNo++, (char*)sendBuffer);
			cnt = 0;
			memset(sendBuffer, 0, ClusterSize);
		}
	}
	if (i % ClusterFatEntries){
		myPart->writeCluster(clNo, (char*)sendBuffer);
	}

}

void FatHandler::load(){
	ClusterNo recBuffer[ClusterFatEntries];
	myPart->readCluster(0, (char*)recBuffer);//ucitamo nulti klaster u bafer

	freeStartCl = recBuffer[0];//od kog KLASTERA nam pocinje lista slobodnih
	rootStartCl = recBuffer[1];//od kog KLASTERA nam pocinje lista podataka
	fatNum = myPart->getNumOfClusters();//recBuffer[2]; broj klastera
	rootEntryNum = recBuffer[3];//broj zapisa na particiji

	ClusterNo i, clNo = 0, cnt = 4;//u fatCache upisujemo podatke iz bafera
	fatCache[0] = recBuffer[cnt++];
	for (i = 1; i < fatNum; i++)
	{
		fatCache[i] = recBuffer[cnt++];
		if (cnt % ClusterFatEntries == 0){//ako se dodje do kraja klastera, citamo sledeci klaster u bafer i nastavljamo
			myPart->readCluster(++clNo, (char*)recBuffer);
			cnt = 0;
		}
	}
}
