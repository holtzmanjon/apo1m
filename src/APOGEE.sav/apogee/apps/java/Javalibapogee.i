/* File : Javalibapogee.i */
%module Javalibapogee

%include "typemaps.i"
%include "arrays_java.i"


/*
 * order of files is very important.
 * base classes must be called out first
 */
%{
#include "DefDllExport.h"
#include "ApogeeCam.h"
#include "CamGen2Base.h"
#include "Alta.h"
#include "ApogeeFilterWheel.h"
#include "Ascent.h"
#include "CameraInfo.h"
#include "CameraStatusRegs.h"
#include "FindDeviceEthernet.h" 
#include "FindDeviceUsb.h" 
#include "Gee.h"
#include "HiC.h" 
#include "Quad.h" 
%}

%include "std_except.i"
/*
 * When a c++ exception occurs, this code will turn it into
 * a Java exception. 
 */
%exception %{
    try {
      $action
    } catch (std::bad_alloc &) {
      return $null;
    } catch (std::exception &e) {
      jclass clazz = jenv->FindClass("java/lang/Exception");
      jenv->ThrowNew(clazz, e.what());
      return $null;
    } catch (...) {
      jclass clazz = jenv->FindClass("java/lang/Exception");
      jenv->ThrowNew(clazz, "Unknown exception");
      return $null;
    }
%}



/* stl support */
%include "std_string.i"
%include "std_vector.i"

/*
 * order of files is very important.
 * base classes must be called out first
 */
/* Let's just grab the original header file here */
%include "DefDllExport.h"
%include "ApogeeCam.h"
%include "CamGen2Base.h"
%include "Alta.h"
%include "ApogeeFilterWheel.h"
%include "Ascent.h"
%include "CameraInfo.h"
%include "CameraStatusRegs.h"
%include "FindDeviceEthernet.h" 
%include "FindDeviceUsb.h" 
%include "Gee.h"
%include "Quad.h" 
%include "HiC.h" 




