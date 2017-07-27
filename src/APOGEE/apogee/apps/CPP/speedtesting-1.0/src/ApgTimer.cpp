/*! 
* 
* Copyright(c) 2011 Apogee Imaging Systems, Inc. 
* \class ApgTimer 
* \brief wrapper for cross platform timing 
* 
*/ 

#include "ApgTimer.h" 

#include "ITimer.h"

#ifdef WIN32
#include "WinTimer.h"
#else
#include "LinuxTimer.h"
#endif

//////////////////////////// 
// CTOR 
ApgTimer::ApgTimer() 
{ 

#ifdef WIN32
    m_timer = std::tr1::shared_ptr<ITimer>( new WinTimer );
#else
    m_timer = std::tr1::shared_ptr<ITimer>( new LinuxTimer );
#endif

} 

//////////////////////////// 
// DTOR 
ApgTimer::~ApgTimer() 
{ 

} 

//////////////////////////// 
// START
void ApgTimer::Start()
{
    m_timer->Start();
}

//////////////////////////// 
// STOP
void ApgTimer::Stop()
{
    m_timer->Stop();
}

//////////////////////////// 
// GET      TIME      IN     MS
double ApgTimer::GetTimeInMs()
{
    return m_timer->GetTimeInMs();
}

//////////////////////////// 
// GET      TIME      IN     SEC
double ApgTimer::GetTimeInSec()
{
    return m_timer->GetTimeInSec();
}