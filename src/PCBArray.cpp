#include "PCBArray.h"
#include "Intr.h"

PCBarray::PCBarray(int size)
{
	lock();
	maxPCBs=size;
	array = new PCB*[maxPCBs];
	for(int i=0; i<maxPCBs; i++)
		array[i]=0;
	currentPCBs=0;
	unlock();
}

int PCBarray::add(PCB* newPCB)
{
	int i;
	for(i=0; i<maxPCBs;i++)	//ako se PCB vec nalazi u nizu
		if(array[i]==newPCB)
			return i;

	lock();
	if(currentPCBs==maxPCBs)
	{
		PCB** temp = new PCB*[maxPCBs+5];
		for(i=0;i<maxPCBs+5;i++)
		{
			if(i<maxPCBs)
			{
				temp[i]=array[i];
				array[i]=0;
			}
			else
				temp[i]=0;
		}
		delete[] array;
		array=temp;
		maxPCBs+=5;
		temp=0;
	}
	for(i=0;i<maxPCBs;i++)
		if(array[i]==0)
			break;
	array[i]=newPCB;
	currentPCBs++;
	unlock();

	return i;
}

PCB* PCBarray::get(int i)
{
	if(i<0 || i>=maxPCBs)
		return 0;
	return array[i];
}

void PCBarray::remove(int i)
{
	if(i<0 || i>=maxPCBs)
		return;
	if(array[i]!=0)
	{
		array[i]=0;
		currentPCBs--;
	}
}

PCBarray::~PCBarray()
{
	lock();
	delete[] array;
	unlock();
}
