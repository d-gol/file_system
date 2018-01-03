#pragma once
#include "part.h"

class Partition;

const unsigned int ZeroClusterFatEntries = 508; //broj ulaza u nultom klasteru
const unsigned int ClusterFatEntries = 512; //broj ulaza u klasteru inace

class FatHandler
{
public:
	FatHandler(Partition*);
	~FatHandler();

	void initFat(); //alocira memoriju za kes
	void format(); //vraca FAT tabelu u pocetno stanje
	void load(); //dovlaci FAT tabelu sa diska (celu)
	void flush(); //vraca celu FAT tabelu na disk

private:

	ClusterNo* fatCache; //niz koji nam predstavlja kesiranu FAT tabelu
	unsigned int clustersTaken; //koliko klastera zauzima FAT tabela na disku?
	ClusterNo freeStartCl; //odakle nam pocinje lista slobodnih?
	ClusterNo rootStartCl; //odakle nam pocinje lista svih fajl zapisa?
	ClusterNo fatNum; //koliko ima ulaza u fat tabeli, odnosno koliko ima klastera na particiji?
	ClusterNo rootEntryNum; //koliko ima zapisa na particiji

	Partition* myPart;

	friend class PartitionHandler;
	friend class KernelFile;
	friend class KernelFS;
};