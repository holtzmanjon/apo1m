#SWIG building
BUILT_SOURCES = $(srcdir)/jlibapogee_wrap.cpp
SWIG_SOURCES = jlibapogee.i

APOGEE_DIR=-I$(top_srcdir)/apogee

# create the swig wrapper - this is done before
# compiling with the BUILT_SOURCES directive
$(srcdir)/jlibapogee_wrap.cpp : $(SWIG_SOURCES)
	$(SWIG) -java -package com.apogee.driver $(APOGEE_DIR) -v -o $@ $<
	
#building the shared library wrapper	
pkgjava_LTLIBRARIES = libjlibapogee.la
libjlibapogee_la_SOURCES = $(srcdir)/jlibapogee_wrap.cpp $(SWIG_SOURCES)
libjlibapogee_la_CPPFLAGS = $(JNI_CPPFLAGS) $(APOGEE_DIR) -v 
libjlibapogee_la_LDFLAGS = -module $(LDFLAGS)

APOGEE_LIB=../../apogee/libapogee.la
libjlibapogee_la_LIBADD = $(APOGEE_LIB)

# BUILD THE JAVA stuff
JAVAROOT = .
pkgjavadir = $(datadir)/java

#info about the jar file
jarfile = jlibapogee.jar
jardir = $(datadir)/java
jar_DATA = $(jarfile)

#java files to be compiled into class files
#via the *JAVA call
dist_noinst_JAVA = AdcSpeed.java \
Alta.java \
AltaF.java\
ApogeeCam.java \
ApogeeFilterWheel.java \
Ascent.java \
Aspen.java \
CameraMode.java \
CameraStatusRegs.java \
CamGen2Base.java \
CoolerStatus.java \
ErrorType.java \
FanMode.java \
FindDeviceEthernet.java \
FindDeviceUsb.java \
HiC.java \
InterfaceType.java \
jlibapogee.java \
jlibapogeeJNI.java \
LedMode.java \
LedState.java \
NetDb.java \
PlatformType.java \
Quad.java \
Resolution.java \
SerialFC.java \
SerialParity.java \
ShutterState.java \
Status.java \
StrDb.java \
SWIGTYPE_p_CameraStatusRegs__AdvStatus.java \
SWIGTYPE_p_CameraStatusRegs__BasicStatus.java \
SWIGTYPE_p_unsigned_char.java \
SWIGTYPE_p_std__vectorT_std__string_t.java \
SWIGTYPE_p_std__vectorT_unsigned_char_t.java \
TriggerMode.java \
TriggerType.java \
Uint16Vector.java

#build the jar file with the output
#*.class files from the *.java files
$(jarfile): $(dist_noinst_JAVA)
	$(JAR) cvf $(JARFLAGS) $@ com

# java and swig generated file are not automatically
# cleaned up
CLEANFILES = $(jarfile) com/apogee/driver/*.class 
