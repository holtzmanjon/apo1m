# General settings
SUBDIRS = linux 

lib_LTLIBRARIES = libapogee.la
libapogee_la_SOURCES = AltaCcdAcqParams.cpp \
AltaCcdAcqParams.h \
Alta.cpp \
AltaData.cpp \
AltaData.h \
AltaEthernetIo.cpp \
AltaEthernetIo.h \
AltaF.cpp \
AltaIo.cpp \
AltaIo.h \
AltaModeFsm.cpp \
AltaModeFsm.h \
AltaUsbIo.cpp \
AltaUsbIo.h \
apgHelper.cpp \
apgHelper.h \
ApgLogger.cpp \
ApgLogger.h \
ApgTimer.cpp \
ApgTimer.h \
ApnCamData.cpp \
ApnCamData.h \
ApnUsbSys.h \
ApogeeCam.cpp \
Ascent.cpp \
AscentBasedIo.cpp \
AscentBasedIo.h \
AscentBasedUsbIo.cpp \
AscentBasedUsbIo.h \
AscentData.cpp \
AscentData.h \
AspenData.cpp \
AspenData.h \
CamCfgMatrix.cpp \
CamCfgMatrix.h \
CameraIo.cpp \
CameraIo.h \
CameraInfo.cpp \
CameraStatusRegs.cpp \
CamGen2Base.cpp \
CamGen2CcdAcqParams.cpp \
CamGen2CcdAcqParams.h \
CamGen2ModeFsm.cpp \
CamGen2ModeFsm.h \
CamHelpers.cpp \
CamHelpers.h \
CamRegMirror.cpp \
CamRegMirror.h \
CamUsbIo.cpp \
CamUsbIo.h \
CcdAcqParams.cpp \
CcdAcqParams.h \
FilterWheelIo.cpp \
FilterWheelIo.h \
FindDeviceEthernet.cpp \
FindDeviceUsb.cpp \
Aspen.cpp \
AspenEthernetIo.cpp \
AspenEthernetIo.h \
AspenIo.cpp \
AspenIo.h \
AspenUsbIo.cpp \
AspenUsbIo.h \
helpers.cpp \
helpers.h \
HiC.cpp \
IAltaSerialPortIo.cpp \
IAltaSerialPortIo.h \
ICamIo.cpp \
ICamIo.h \
ILog.cpp \
ILog.h \
ImgFix.cpp \
ImgFix.h \
ITimer.cpp \
ITimer.h \
IUsb.cpp \
IUsb.h \
libCurlWrap.cpp \
libCurlWrap.h \
LoggerException.cpp \
LoggerException.h \
ModeFsm.cpp \
ModeFsm.h \
parseCfgTabDelim.cpp \
parseCfgTabDelim.h \
PlatformData.cpp \
PlatformData.h \
UdpSocketBase.cpp \
UdpSocketBase.h \
ApogeeFilterWheel.cpp \
PromFx2Io.cpp \
PromFx2Io.h \
Quad.cpp

libapogee_la_includedir=$(includedir)/libapogee-@LIBAPOGEE_VERSION_DIR@/apogee

libapogee_la_include_HEADERS = ApogeeCam.h \
Alta.h \
AltaF.h \
Ascent.h \
CamGen2Base.h \
CameraInfo.h \
CameraStatusRegs.h \
Aspen.h \
FindDeviceEthernet.h \
FindDeviceUsb.h \
HiC.h \
ApogeeFilterWheel.h \
Quad.h \
DefDllExport.h \
versionNo.h \
doc.h

# Specify the libraries to compile our library and the accompanying binaries (change these according to your needs)
libapogee_la_LIBADD = -lcurl -lusb-1.0 -lboost_regex-mt linux/libapogeelinux.la

#this works on linux
libapogee_la_LDFLAGS = $(BOOST_LDFLAGS) -release @LIBAPOGEE_RELEASE_VERSION@ -version-info @LIBAPOGEE_SO_VERSION@

# for mac may need to remove the version items, because of how OSX deals
# with them is different
#libapogee_la_LDFLAGS = $(BOOST_LDFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/apogee/linux $(BOOST_CPPFLAGS) -DSYSCONFDIR="\"@sysconfdir@\""


