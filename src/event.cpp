#include "event.h"
#include "PCB.h"
#include "Intr.h"
#include "KernEv.h"

Event::Event (IVTNo ivtNo)
{
	lock();
	myImpl = new KernelEv(ivtNo);
	unlock();
}

Event::~Event ()
{
	lock();
	delete myImpl;
	unlock();
}

void Event::wait ()
{
	myImpl->wait();
}

void Event::signal()
{
	myImpl->signal();
}
