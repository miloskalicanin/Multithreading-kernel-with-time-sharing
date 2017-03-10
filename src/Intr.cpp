
#include "Intr.h"
#include "PCB.h"
#include "SCHEDULE.H"
#include "KernSem.h"
#include <dos.h>


volatile unsigned interruptType=0;
/*
 * 0 - nema zahteva za promenu konteksta
 * 1 - timer
 * 2 - dispatch
 */
volatile int timeCounter = 0;

volatile unsigned locked = 0;

//pomocne promenljive
volatile unsigned tsp,tss,tbp;



class IdleTherad : public Thread
{
public:
	void run()
	{
		while(PCB::runningThreads>=0);
	}
};


IdleTherad* idle = 0;	//idle nit

void lock()
{
#ifndef BCC_BLOCK_IGNORE
		asm pushf
		asm cli
#endif
		locked++;
#ifndef BCC_BLOCK_IGNORE
		asm popf
#endif
}

void unlock()
{
#ifndef BCC_BLOCK_IGNORE
	asm{
		pushf
		cli
	}
#endif
	locked--;
	if(locked<0)
		locked=0;
#ifndef BCC_BLOCK_IGNORE
	asm popf
#endif
	if(interruptType!=0)
		timer();
}


void tick();

int oldRoutine=0;

void interrupt timer(...)
{
	if (PCB::running->timeSlice>0)
	{
		if(interruptType==0)
			timeCounter--;
		if (timeCounter <= 0 && interruptType==0)
		{
			interruptType=1; //prekid od timera
		}
	}


	if(interruptType==0 || interruptType==1) //ako timer nije eksplicitno pozvan (desio se otkucaj)
	{										//potrebno je skociti na staru prekidnu rutinu
		oldRoutine=1;
		KernelSem::checkTime();
	}

	if(interruptType!=0) //postoji zahtev za promenu konteksta
	{
		if(locked==0)	//dozvoljena promena konteksta
		{
#ifndef BCC_BLOCK_IGNORE
			asm{
				mov tsp, sp
				mov tss, ss
				mov tbp, bp
			}
#endif

			PCB::running->sp = tsp;
			PCB::running->ss = tss;
			PCB::running->bp = tbp;

			if(PCB::running->status==0)
				Scheduler::put((PCB*)PCB::running);

			PCB::running = Scheduler::get();

			if(PCB::running == 0)
				PCB::running = PCB::get_PCB_by_ID(idle->getId());

			tsp = PCB::running->sp;
			tss = PCB::running->ss;
			tbp = PCB::running->bp;

			timeCounter = PCB::running->timeSlice;

#ifndef BCC_BLOCK_IGNORE
			asm {
				mov sp, tsp
				mov ss, tss
				mov bp, tbp
			}
#endif
			interruptType=0;
		}
	}

	// poziv stare prekidne rutine koja se nalazila na 08h, a sad je na 60h
    // poziva se samo kada nije zahtevana promena konteksta – tako da se
    // stara rutina poziva samo kada je stvarno doslo do prekida od timera
	if(oldRoutine)
	{
		oldRoutine=0;
		tick();
#ifndef BCC_BLOCK_IGNORE
		asm int 60h;
#endif
	}
	
	if(interruptType>1)		
		interruptType=1;
	//ako je bio pozvan dispatch i locked==1
	//potrebno je dozvoliti prekide od timera
	//pri cemu je obelezeno da je zahtevana promena konteksta
	//(bice dozvoljeno pozivanje stare prekidne rutine pri otkucaju timera)
}



typedef void interrupt (*pInterrupt)(...);

pInterrupt oldTimer;



// postavlja novu prekidnu rutinu
void inicTimer()
{
	lock();
#ifndef BCC_BLOCK_IGNORE

	oldTimer = getvect(0x8);
	setvect(0x8, timer);
	setvect(0x60, oldTimer);

#endif

	//stvaranje idle i main niti
	idle=new IdleTherad();
	PCB* p = PCB::get_PCB_by_ID(idle->getId());
	p->status = 3;
	p->started = 1;
	PCB::running = new PCB(0);
	PCB::running->started=1;
	PCB::runningThreads++;
	unlock();
}


// vraca staru prekidnu rutinu
void restoreTimer()
{
	lock();
#ifndef BCC_BLOCK_IGNORE

	setvect(0x8, oldTimer);
	oldTimer=0;

#endif

	//brisanje idle i main niti
	PCB* p = PCB::get_PCB_by_ID(idle->getId());
	p->status = 1; //gotovo
	delete idle;
	delete PCB::running;
	PCB::running = 0;
	PCB::runningThreads--;
	unlock();
}
