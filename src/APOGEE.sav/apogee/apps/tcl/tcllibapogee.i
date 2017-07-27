/* File : tcllibapogee.i */
%module tcllibapogee

%include "typemaps.i"
%include "cstring.i"
%include "carrays.i"
%include "cdata.i"

%cstring_bounded_output(char *rtnStr, 256)
%array_class(unsigned short , ushortArray);

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
typedef short unsigned int uint16_t;
%}

%include "exception.i"
/*
 * When a c++ exception occurs, this code will turn it into
 * a tcl exception. 
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
%include "std_string.i"
%include "std_vector.i"

%{
#define SWIG_FILE_WITH_INIT
%}
%init %{
%}




/* for functions that pass data out by reference, specifically the GetImage function */
%typemap(in,numinputs=0) 
  (std::vector<uint16_t> & INOUT) 
  (std::vector<uint16_t> data) {
    $1 = &data;
}


%apply std::vector<uint16_t> & INOUT { std::vector<uint16_t>& out };

%typemap(in) uint16_t = int;
%typemap(out) uint16_t = int;

/*
 * order of files is very important.
 * base classes must be called out first
 */
/* Let's just grab the original header file here */
typedef short unsigned int uint16_t;
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




