#include "KernEv.h"
#include "Intr.h"
#include "SCHEDULE.H"
#include <dos.h>


KernelEv* IVTEntry::events[256] = {0};
IVTEntry* IVTEntry::entries[256] = {0};


KernelEv::KernelEv(IVTNo ivtNo)
{
	lock();
	val=0;
	owner=(PCB*)PCB::running;
	this->ivtNo=ivtNo;
	IVTEntry::events[ivtNo]=this;
#ifndef BCC_BLOCK_IGNORE
	old = getvect(ivtNo);
	setvect(ivtNo, IVTEntry::entries[ivtNo]->newRoutine);
#endif
	unlock();
}

KernelEv::~KernelEv()
{
	lock();
	IVTEntry::events[ivtNo]=0;
#ifndef BCC_BLOCK_IGNORE
	setvect(ivtNo, old);
#endif
	unlock();
}

void KernelEv::wait ()
{
	lock();
	if(PCB::running==owner)
	{
		if(val==0)
		{	//block
			owner->status=2;
			unlock();
			dispatch();
		}
		else
		{
			val=0;
			unlock();
		}
	}
}

void KernelEv::signal()
{
	lock();
	if(val==0)
	{
		if(owner->status==2)
		{	//deblock
			owner->status=0;
			Scheduler::put(owner);
			unlock();
			dispatch();
		}
		else
		{
			val=1;
			unlock();
		}
	}
	else
		unlock();
}








IVTEntry::IVTEntry(pInterrupt newRout, IVTNo ivtno)
{
	entries[ivtno]=this;
	newRoutine=newRout;
	ivtNo=ivtno;
}

IVTEntry::~IVTEntry()
{
	entries[ivtNo]=0;
}

void IVTEntry::signal()
{
	if(events[ivtNo])
		events[ivtNo]->signal();
}

void IVTEntry::callOld()
{
	if(events[ivtNo])
		events[ivtNo]->old();
}

