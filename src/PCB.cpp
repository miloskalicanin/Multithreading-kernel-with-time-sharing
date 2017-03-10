#include "PCB.h"
#include "thread.h"
#include "SCHEDULE.H"
#include "PCBArray.h"
#include "Intr.h"
#include <dos.h>


volatile PCB* PCB::running=0;
volatile unsigned PCB::runningThreads = 0;
PCBarray PCBarr = PCBarray(5); //aktivni objekti PCB se smestaju u ovaj niz

PCB::PCB(Thread* thr, unsigned long stackSz, unsigned int t)
{
#ifndef BCC_BLOCK_IGNORE
	asm pushf
	asm cli
#endif
	myThread=thr;
	status=0;
	started=0;
	initialize(stackSz);
	timeSlice=t;
	children=blocked=0;
	myID=PCBarr.add(this);
#ifndef BCC_BLOCK_IGNORE
	asm popf
#endif
}

PCB::PCB(unsigned int t)
{
#ifndef BCC_BLOCK_IGNORE
	asm pushf
	asm cli
#endif
	myThread=0;
	stack=0;
	stSz=0;
	ss=sp=bp=0;
	timeSlice=t;
	status=0;
	started=1;
	children=blocked=0;
	myID=PCBarr.add(this);
#ifndef BCC_BLOCK_IGNORE
	asm popf
#endif
}

PCB::~PCB()
{
	lock();
	if(stack!=0)
		delete[] stack;
	stack=0;
	myThread=0;
	removeFromArray();
	unlock();
}

void PCB::initialize(unsigned long stackSz)
{
	stSz = stackSz;
	if(stSz>65535)
		stSz=65535;
	stSz/=sizeof(unsigned);
	stack = new unsigned[stSz];
	stack[stSz-1] =0x200;//setovan I fleg u pocetnom PSW-u za nit
#ifndef BCC_BLOCK_IGNORE
	stack[stSz-2] = FP_SEG(PCB::wrapper);
	stack[stSz-3] = FP_OFF(PCB::wrapper);
	sp = FP_OFF(stack+stSz-12); //svi sacuvani registri
									 //pri ulasku u interrupt rutinu
	ss = FP_SEG(stack+stSz-12);
	bp = sp;
	stack[stSz-12]=FP_OFF(stack+stSz);	//pocetna vrednost bp registra ce pokazivati
										//na prvu lokaciju posle niza stack
#endif
}

void PCB::wrapper()
{
	PCB::running->myThread->run();
	((PCB*)PCB::running)->exitPCB();
}


void PCB::exitPCB()
{
	//Thread::waitForForkChildren();
	lock();
	PCB* temp;
	List* l;
	while(blocked)
	{
		temp=blocked->p;
		temp->status=0;	//postaje aktivna
		Scheduler::put(temp);
		l=blocked;
		blocked=blocked->next;
		delete l;
	}
	PCB::running->status=1;
	runningThreads--;
	unlock();

	dispatch();
}

PCB* PCB::get_PCB_by_ID(int id)
{
	if(id<0)
		return 0;
	return PCBarr.get(id);
}


void PCB::removeFromArray()
{
	lock();
	if(myID>=0)
		PCBarr.remove(myID);
	myID=-1;
	unlock();
}

