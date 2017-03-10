#include "KernSem.h"
#include "Intr.h"
#include "PCB.h"
#include "SCHEDULE.H"
#include <dos.h>


volatile KernelSem::List* KernelSem::globalBlocked=0;

KernelSem::KernelSem(Semaphore *sem, int value)
{
	mySem=sem;
	val=value;
	blockedHead=blockedTail=0;
}

KernelSem::~KernelSem()
{
	List *temp = blockedHead;
	while(temp)
	{
		deblock(temp);
		temp=blockedHead;
	}
}

int KernelSem::wait(Time maxTimeToWait)
{
	lock();
	if(--val<0)
	{
		List *l = new List();
		volatile unsigned ret=0;

		if(maxTimeToWait>0)
		{
			List *g = new List();
			g->next=(List*)globalBlocked;
			globalBlocked=g;
			g->time=maxTimeToWait;
			g->pcb=(PCB*)PCB::running;
			g->sem=this;
			g->retValue=&ret;
		}

		l->pcb=(PCB*)PCB::running;
		l->next=0;
		l->time=maxTimeToWait;
		l->sem=this;
		l->retValue=&ret;
		PCB::running->status=2;
		if(blockedHead==0)
			blockedHead=l;
		else
			blockedTail->next=l;
		blockedTail=l;
		unlock();

		dispatch();

		deblock(l);

		if(ret==2)
			return 0;	//ako je probudjen iz timera
		else
			return 1;	//probudjen pomocu signal
	}
	else
	{
		unlock();
		return 1; //nit nije blokirana
	}
}

void KernelSem::signal()
{
	lock();
	if(val++<0 && blockedHead!=0)
	{
		List *l = blockedHead;
		while(l && (*(l->retValue)))
			l=l->next;
		if(l!=0)
		{
			l->pcb->status=0;
			*(l->retValue)=1;
			Scheduler::put(l->pcb);
		}
	}
	unlock();
}


void KernelSem::deblock(List* l)
{	 //argument l je element lokalne liste objekta semafora, ne sme biti element globalne liste
	lock();

	List *p = blockedHead, *q=0;
	PCB* pcb = l->pcb;
	unsigned toCheck = l->time>0; //vrednost 1 ako se nalazi i u listi globalnih


	while(p->pcb!=l->pcb)
	{
		q=p;
		p=p->next;
	}

	if(q==0)
	{
		blockedHead=p->next;
		delete p;
		if(blockedHead==0)
			blockedTail=0;
	}
	else
	{
		if(p==blockedTail)
			blockedTail=q;
		q->next = p->next;
		delete p;
	}

	if(toCheck)	//ako se nalazi i u globalnoj listi
	{
		p=(List*)globalBlocked;
		q=0;
		while(p->pcb!=pcb)
		{
			q=p;
			p=p->next;
		}

		if(q==0)
		{
			globalBlocked=p->next;
			delete p;
		}
		else
		{
			q->next = p->next;
			delete p;
		}
	}

	unlock();
}

void KernelSem::checkTime()
{
	List* temp = (List*)globalBlocked;
	while(temp)
	{
		temp->time--;
		if(temp->time==0 && (*(temp->retValue))==0)
		{
			temp->sem->val++;
			temp->pcb->status=0;
			*(temp->retValue)=2;
			Scheduler::put(temp->pcb);
		}
		temp=temp->next;
	}
}
