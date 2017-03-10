#ifndef KERNSEM_H_
#define KERNSEM_H_

#include "PCB.h"
#include "semaphor.h"

class KernelSem
{
public:

	class List
	{
		public:
			PCB* pcb;
			List* next;
			volatile Time time;
			volatile unsigned* retValue;
			KernelSem *sem;
	};

	List *blockedHead, *blockedTail; //blokirane niti na semaforu
	static volatile List* globalBlocked; //globalna lista svih blokiranih osetljivih na vreme
	Semaphore *mySem;
	int val;

	KernelSem(Semaphore *sem, int value);
	~KernelSem();
	int wait(Time maxTimeToWait);
	void signal();
	void deblock(List*);
	static void checkTime();
};


#endif
