/*! 
* 
* Copyright(c) 2011 Apogee Imaging Systems, Inc. 
* \class ITimer 
* \brief interface for cross platform timer 
* 
*/ 


#ifndef ITIMER_INCLUDE_H__ 
#define ITIMER_INCLUDE_H__ 


class ITimer 
{ 
    public: 
        virtual ~ITimer(); 
        virtual void Start() = 0;
        virtual void Stop() = 0;

        virtual double GetTimeInMs() = 0;
        virtual double GetTimeInSec() = 0;
}; 

#endif