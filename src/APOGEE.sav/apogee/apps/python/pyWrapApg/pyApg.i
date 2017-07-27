/* File : pyApg.i */
%module pyApg

%{
#include "DefDllExport.h"
#include "ApogeeCam.h" 
#include "Alta.h" 
#include "TestCamAlta.h" 
#include "CamGen2Base.h" 
#include "Ascent.h" 
#include "TestCamAscent.h" 
#include "Aspen.h" 
#include "TestCamAspen.h" 
#include "Quad.h" 
#include "TestCamQuad.h" 
#include "HiC.h" 
#include "TestCamHiC.h" 
#include "AltaF.h" 
#include "TestCamAltaF.h" 
#include "TestCamHelpers.h" 
#include "CameraInfo.h" 
#include "ApogeeFilterWheel.h"
#include "TestFilterWheel.h"
#include "FindDeviceEthernet.h" 
#include "FindDeviceUsb.h" 
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

/* numpy include for vectors */
%{
#define SWIG_FILE_WITH_INIT
%}
%include "numpy.i"
%init %{
import_array();
%}



/* for functions that pass data out by reference, specifically the GetImage function */
%typemap(in,numinputs=0) 
  (std::vector<uint16_t> & INOUT) 
  (std::vector<uint16_t> data) {
    $1 = &data;
}

%typemap(argout) (std::vector<uint16_t> & INOUT) {
 npy_intp length = data$argnum.size();
 $result = PyArray_SimpleNew(1, &length, PyArray_USHORT);
 memcpy(PyArray_DATA($result),&data$argnum[0],sizeof(uint16_t)*data$argnum.size());
}

%apply std::vector<uint16_t> & INOUT { std::vector<uint16_t>& out };

/* for functions that return std::vector<uint16_t> */
%typemap(out) std::vector<uint16_t> {
 npy_intp length = $1.size();
 $result = PyArray_SimpleNew(1, &length, PyArray_USHORT);
 memcpy(PyArray_DATA($result),&((*(&$1))[0]),sizeof(uint16_t)*$1.size());
}

/* disable the default ctors for the test cams*/
%nodefaultctor TestCamAlta;
%nodefaultctor TestCamAscent;
%nodefaultctor TestCamGee;
%nodefaultctor TestCamQuad;

/* Let's just grab the original header file here */
%include "DefDllExport.h"
%include "ApogeeCam.h"
%include "Alta.h" 
%include "TestCamAlta.h" 
%include "CamGen2Base.h" 
%include "Ascent.h" 
%include "TestCamAscent.h" 
%include "Aspen.h" 
%include "TestCamAspen.h" 
%include "Quad.h" 
%include "TestCamQuad.h" 
%include "HiC.h" 
%include "TestCamHiC.h" 
%include "AltaF.h" 
%include "TestCamAltaF.h" 
%include "TestCamHelpers.h" 
%include "CameraInfo.h" 
%include "ApogeeFilterWheel.h"
%include "TestFilterWheel.h"
%include "FindDeviceEthernet.h" 
%include "FindDeviceUsb.h"




