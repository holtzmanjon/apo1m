/* File : jlibapogee.i */
%module jlibapogee

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
#include "AltaF.h"
#include "CameraInfo.h"
#include "CameraStatusRegs.h"
#include "FindDeviceEthernet.h" 
#include "FindDeviceUsb.h" 
#include "Aspen.h"
#include "Quad.h" 
#include "HiC.h" 
%}

%include "exception.i"
/*
 * When a c++ exception occurs, this code will turn it into
 * a python exception. 
 */
%exception {
    try {
        $action
    } catch(const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what() );
    } catch(...) {
        SWIG_exception(SWIG_RuntimeError, "Unknown error");
    }
}


/* stl support */
%include "stdint.i"
%include "std_string.i"
%include "std_vector.i"

namespace std {
   %template(Uint16Vector) vector<uint16_t>;
}

/* for mapping unsigned short reference to int[] */

%include <typemaps.i>
%apply int *OUTPUT { uint16_t & };

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
%include "AltaF.h"
%include "CameraInfo.h"
%include "CameraStatusRegs.h"
%include "FindDeviceEthernet.h" 
%include "FindDeviceUsb.h" 
%include "Aspen.h"
%include "Quad.h"
%include "HiC.h"