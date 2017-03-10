//File PCB.h
#ifndef _PCB_H_
#define _PCB_H_

#include "thread.h"

class PCB
{
public:
		class List
		{
		public:
			PCB* p;
			List* next;
			List(PCB* pp, List* ss)
			{
				p=pp;
				next=ss;
			}
		};
		void initialize(unsigned long stackSz);
		friend class Thread;

		unsigned sp;
		unsigned ss;
		unsigned bp;
		unsigned* stack;	//pokazivac na stek
		StackSize stSz;		//broj mem. lokacija steka
		unsigned started;
		unsigned status;
		/*
		 * 0 - ready
		 * 1 - finished
		 * 2 - blocked
		 * 3 - iddle thread
		 * 4 - exited (nasilno ugasena metodom Thread::exit())
		 */
		int timeSlice;

		ID myID;
		Thread* myThread;
		List* blocked, *children; //lista niti koje cekaju trenutnu i niti dece

		static volatile PCB* running;
		static volatile unsigned runningThreads;

		static void wrapper();
		static PCB* get_PCB_by_ID(int id);

		PCB(Thread* thr, unsigned long stackSz, unsigned int time);
		PCB(unsigned int time);
		~PCB();
		void exitPCB();
		void removeFromArray();
};


#endif
