/*! 
* 
* Copyright(c) 2011 Apogee Imaging Systems, Inc. 
* \class LinuxTimer 
* \brief linux implementation of the timmer 
* 
*/ 

#ifndef LINUXTIMER_INCLUDE_H__ 
#define LINUXTIMER_INCLUDE_H__ 

#include <sys/time.h>
#include "ITimer.h"

class LinuxTimer : public ITimer
{ 
    public: 
        LinuxTimer(); 
        virtual ~LinuxTimer(); 

        void Start();
        void Stop();

        double GetTimeInMs();
        double GetTimeInSec();

    private:
        struct timeval m_start;
		struct timeval m_end;
}; 

#endif
