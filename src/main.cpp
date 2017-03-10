#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include "Intr.h"


#include "mainThr.h"

//int userMain(int argc, char* argv[]);


int main(int argc, char* argv[])
{

#ifndef BCC_BLOCK_IGNORE
		asm pushf
		asm cli
#endif
	inicTimer();
#ifndef BCC_BLOCK_IGNORE
		asm popf
#endif


	//int i = userMain(argc,argv);
	int i;

	lock();

	mainThr* t1 = new mainThr(8192,15,&i,argc,argv);
	t1->start();

	unlock();

	t1->waitToComplete();

		

#ifndef BCC_BLOCK_IGNORE
		asm pushf
		asm cli
#endif
	restoreTimer();
#ifndef BCC_BLOCK_IGNORE
		asm popf
#endif

	return i;
}
