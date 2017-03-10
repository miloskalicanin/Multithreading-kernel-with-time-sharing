#include "semaphor.h"
#include "KernSem.h"
#include "Intr.h"


Semaphore::Semaphore(int init)
{
	lock();
	myImpl = new KernelSem(this, init);
	unlock();
}

Semaphore::~Semaphore()
{
	lock();
	delete myImpl;
	unlock();
}

int Semaphore::wait(Time maxTimeToWait)
{
	int ret;
#ifndef BCC_BLOCK_IGNORE
		asm pushf
		asm cli
#endif
	ret = myImpl->wait(maxTimeToWait);
#ifndef BCC_BLOCK_IGNORE
		asm popf
#endif
	return ret;
}

void Semaphore::signal()
{
#ifndef BCC_BLOCK_IGNORE
		asm pushf
		asm cli
#endif
	myImpl->signal();
#ifndef BCC_BLOCK_IGNORE
		asm popf
#endif
}

int Semaphore::val() const
{
	return myImpl->val;
}
