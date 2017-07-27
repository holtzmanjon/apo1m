noinst_PROGRAMS = BulkSequence \
BulkTDI \
ExternalShutter \
ExternalTrigger \
FilterWheel \
Kinetics \
MultiCamera \
SimpleExposure \
StreamingTDI \
AscentExample \
AspenExample

APOGEE_LIB=../apogee/libapogee.la
BulkSequence_SOURCES = BulkSequence.cpp apgSampleCmn.cpp apgSampleCmn.h
BulkSequence_LDADD = $(APOGEE_LIB)

BulkTDI_SOURCES = BulkTDI.cpp apgSampleCmn.cpp apgSampleCmn.h
BulkTDI_LDADD = $(APOGEE_LIB)

ExternalShutter_SOURCES = ExternalShutter.cpp apgSampleCmn.cpp apgSampleCmn.h
ExternalShutter_LDADD = $(APOGEE_LIB)

ExternalTrigger_SOURCES = ExternalTrigger.cpp apgSampleCmn.cpp apgSampleCmn.h
ExternalTrigger_LDADD = $(APOGEE_LIB)

FilterWheel_SOURCES = FilterWheel.cpp apgSampleCmn.cpp apgSampleCmn.h
FilterWheel_LDADD = $(APOGEE_LIB)

Kinetics_SOURCES = Kinetics.cpp apgSampleCmn.cpp apgSampleCmn.h
Kinetics_LDADD = $(APOGEE_LIB)

MultiCamera_SOURCES = MultiCamera.cpp apgSampleCmn.cpp apgSampleCmn.h
MultiCamera_LDADD = $(APOGEE_LIB)

SimpleExposure_SOURCES = SimpleExposure.cpp apgSampleCmn.cpp apgSampleCmn.h
SimpleExposure_LDADD = $(APOGEE_LIB)

StreamingTDI_SOURCES = StreamingTDI.cpp apgSampleCmn.cpp apgSampleCmn.h
StreamingTDI_LDADD = $(APOGEE_LIB)

AscentExample_SOURCES =  AscentExample.cpp apgSampleCmn.cpp apgSampleCmn.h
AscentExample_LDADD = $(APOGEE_LIB)

AspenExample_SOURCES =  AspenExample.cpp apgSampleCmn.cpp apgSampleCmn.h
AspenExample_LDADD = $(APOGEE_LIB)

AM_CPPFLAGS = -I$(top_srcdir)
