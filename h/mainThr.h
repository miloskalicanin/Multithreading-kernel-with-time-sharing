#ifndef _mainThr_H_
#define _mainThr_H_

#include "PCB.h"
#include "Intr.h"
#include <iostream.h>
#include <stdio.h>
#include "semaphor.h"
#include "thread.h"


int userMain(int argc, char* argv[]);

class mainThr:public Thread
{
private:
	unsigned long stek;
	unsigned int time;
	int argc;
	char** argv;
	int *ret;
public:
	mainThr(unsigned long s = 4096, unsigned int t = defaultTimeSlice,int *ret,int argc, char* argv[]) : Thread(s,t)
	{
		stek=s;
		time=t;
		this->argc=argc;
		this->argv=argv;
		this->ret=ret;
	}

	Thread* clone() const
	{
		return new mainThr(stek,time,ret,argc,argv);
	}

	virtual void run()
	{
		*ret=userMain(argc,argv);
	}
};


#endif
