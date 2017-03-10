#include "thread.h"
#include "PCB.h"
#include "Intr.h"
#include "SCHEDULE.H"
#include <dos.h>


Thread::Thread(StackSize stackSize, Time timeSlice)
{
	lock();
	myPCB = new PCB(this,stackSize,timeSlice);
	unlock();
}

void Thread::start()
{
	lock();
	if(myPCB->started==0)
	{
		Scheduler::put(myPCB);
		myPCB->started=1;
		PCB::runningThreads++;
	}
	unlock();
}

void Thread::waitToComplete()
{
	if(myPCB->status!=1 && myPCB->status!=4)
	{
		lock();
		PCB::List *temp = new PCB::List((PCB*)PCB::running,myPCB->blocked);
		myPCB->blocked=temp;
		PCB::running->status=2; //nit je blokirana
		unlock();

		dispatch();
	}
}

Thread::~Thread()
{
	waitToComplete();
	lock();
	delete myPCB;
	unlock();
}

ID Thread::getId()
{
	return myPCB->myID;
}

ID Thread::getRunningId()
{
	return PCB::running->myID;
}

Thread* Thread::getThreadById(ID id)
{
	return (PCB::get_PCB_by_ID(id))->myThread;
}


extern volatile unsigned interruptType;
 void dispatch()
 {
#ifndef BCC_BLOCK_IGNORE
	 asm pushf
	 asm cli
#endif
	 interruptType=2;
	 timer();
#ifndef BCC_BLOCK_IGNORE
	 asm popf
#endif
 }




void Thread::exit()
{
	((PCB*)PCB::running)->exitPCB(); 	//ovo se moze izostaviti, ako ne zelimo da niti koje cekaju
										//na nasilno ugasenu nit nastave sa izvrsavanjem
	PCB::running->status=4;

	dispatch();
}

void Thread::waitForForkChildren()
{
	PCB::List *temp = PCB::running->children;
	while(temp)
	{
		temp->p->myThread->waitToComplete();
		temp=temp->next;
	}
}

Thread* Thread::clone() const
{
	return 0;
}


//pomocne promenljive
volatile PCB* parent, *childPCB;
volatile Thread *childThread;
volatile StackSize copyCounter;
volatile unsigned tsp_, tss_, tbp_;
long int temp1, temp2, temp3, disp;


void interrupt copyStack()
{
	//kopiranje steka
	for(copyCounter=0; copyCounter<parent->stSz;copyCounter++)
		if(PCB::running==parent)
			childPCB->stack[copyCounter]=parent->stack[copyCounter];
		else
			break;

	if(PCB::running==parent)
	{
	#ifndef BCC_BLOCK_IGNORE
		asm{
			mov tsp_, sp
			mov tss_, ss
			mov tbp_, bp
		}

		 temp1 = 0x10*(unsigned long)tss_+(unsigned long)tbp_;
		 temp2 = 0x10*(unsigned long)FP_SEG(parent->stack)+(unsigned long)FP_OFF(parent->stack);

		 //pomeraj base pointera u odnosu na pocetak steka
		 disp = (temp1 - temp2)/sizeof(unsigned);


		 childPCB->ss=FP_SEG(childPCB->stack + disp);
		 childPCB->sp=FP_OFF(childPCB->stack + disp);
		 childPCB->bp=childPCB->sp;
	#endif

		tbp_=parent->stack[disp];
		temp3=disp;
		temp1 = 0x10*(unsigned long)tss_+(unsigned long)tbp_;
		disp = (temp1 - temp2)/sizeof(unsigned);

		while(disp<parent->stSz)
		 {
		#ifndef BCC_BLOCK_IGNORE
			childPCB->stack[temp3] = FP_OFF(childPCB->stack + disp);
		#endif

			tbp_=parent->stack[disp];
			temp3=disp;
			temp1 = 0x10*(unsigned long)tss_+(unsigned long)tbp_;
			disp = (temp1 - temp2)/sizeof(unsigned);
		 }

		/*
		 *	podesiti pocetni bp da bude 0
		 
		 	tbp_=parent->stack[disp];

			while(tbp_)
			{
				temp3=disp;
				temp1 = 0x10*(unsigned long)tss_+(unsigned long)tbp_;
				disp = (temp1 - temp2)/sizeof(unsigned);

	#ifndef BCC_BLOCK_IGNORE
				childPCB->stack[temp3] = FP_OFF(childPCB->stack + disp);
	#endif

				tbp_=parent->stack[disp];
		 	}
			
		 *
		 */

	}
}



ID Thread::fork()
{
	lock();
	parent = PCB::running;

	if(PCB::running->myThread && PCB::running->stack)
		childThread = PCB::running->myThread->clone();
	else
		return -1;

	if(childThread==0 || childThread->myPCB->stack==0)
		return -1;

	childPCB=childThread->myPCB;

	PCB::List *temp = new PCB::List((PCB*)childPCB,parent->children);
	parent->children=temp;

	copyStack();

	if(PCB::running==parent)
	{
		((Thread*)childThread)->start();
		unlock();
		return ((Thread*)childThread)->getId();
	}
	else
	{
		return 0;
	}
}

