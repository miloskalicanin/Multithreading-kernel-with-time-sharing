#ifndef _PCB_ARRAY_
#define _PCB_ARRAY_

#include "PCB.h";

class PCB;

class PCBarray
{
	private:
		PCB** array;
		int currentPCBs, maxPCBs;
	public:
		PCBarray(int size);
		int add(PCB* newPCB);
		PCB* get(int i);
		void remove(int i);
		~PCBarray();
};

#endif
