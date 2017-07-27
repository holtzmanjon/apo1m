BUILT_SOURCES = $(srcdir)/pylibapogee_wrap.cpp
SWIG_SOURCES = pylibapogee.i

APOGEE_DIR=-I$(top_srcdir)/apogee

EXTRA_DIST = numpy.i

pylibapogee_PYTHON = pylibapogee.py __init__.py pylibapogee_setup.py 
pylibapogee_LTLIBRARIES = _pylibapogee.la
pylibapogeedir = $(pythondir)/pylibapogee

_pylibapogee_la_SOURCES = $(srcdir)/pylibapogee_wrap.cpp $(SWIG_SOURCES)
_pylibapogee_la_CPPFLAGS = $(AX_SWIG_PYTHON_CPPFLAGS) $(APOGEE_DIR) -v 
_pylibapogee_la_LDFLAGS = -module $(LDFLAGS)

APOGEE_LIB=../../apogee/libapogee.la
_pylibapogee_la_LIBADD = $(APOGEE_LIB)


$(srcdir)/pylibapogee_wrap.cpp : $(SWIG_SOURCES)
	$(SWIG) $(AX_SWIG_PYTHON_OPT) $(APOGEE_DIR) -threads -v -o $@ $<