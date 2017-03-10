#ifndef KERNEV_H_
#define KERNEV_H_

#include "event.h"
#include "PCB.h"
#include <dos.h>

typedef void interrupt (*pInterrupt)(...);


class KernelEv
{
	public:
		IVTNo ivtNo;
		pInterrupt old;
		PCB *owner;
		volatile int val;

		KernelEv(IVTNo ivtNo);
		~KernelEv();
		void wait();
		void signal();
};



class IVTEntry
{
public:
	static KernelEv *events[256];
	static IVTEntry *entries[256];
	IVTEntry(pInterrupt newRoutine, IVTNo ivtno);
	~IVTEntry();
	void signal();
	void callOld();
	pInterrupt newRoutine;
	IVTNo ivtNo;
};


#define PREPAREENTRY(_num,_callOld);\
	void interrupt inter##_num(...){\
	if(IVTEntry::entries[(unsigned char)_num]){\
		(IVTEntry::entries[(unsigned char)_num])->signal();\
		if(_callOld==1) (IVTEntry::entries[(unsigned char)_num])->callOld();\
	}}\
	IVTEntry newIVTEntry##num((pInterrupt)inter##_num,(unsigned char)_num);


#endif /* KERNEV_H_ */
