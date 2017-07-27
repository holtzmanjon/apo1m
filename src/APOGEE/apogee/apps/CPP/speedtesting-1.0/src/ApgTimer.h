/*! 
* 
* Copyright(c) 2011 Apogee Imaging Systems, Inc. 
* \class ApgTimer 
* \brief wrapper for cross platform timing 
* 
*/ 
#ifndef APGTIMER_INCLUDE_H__ 
#define APGTIMER_INCLUDE_H__ 

#ifdef WIN32
#include <memory>
#else
#include <tr1/memory>
#endif

class ITimer;

class ApgTimer 
{ 
    public: 
        ApgTimer(); 
        virtual ~ApgTimer(); 

        void Start();
        void Stop();

        double GetTimeInMs();
        double GetTimeInSec();

    private:
        std::tr1::shared_ptr<ITimer> m_timer;
}; 

#endif