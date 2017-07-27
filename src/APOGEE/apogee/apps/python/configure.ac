#General stuff
AC_INIT(pyLibapogee, 0.0.1, support@ccd.com)
AM_INIT_AUTOMAKE(pyLibapogee, 0.0.1)
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

LT_INIT([dlopen])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PATH_PYTHON(2.6)
AX_PKG_SWIG(1.3.40)
AX_SWIG_ENABLE_CXX
AX_SWIG_PYTHON

# Checks for standard header files.
AC_CHECK_HEADERS([stdint.h])

#list the makefiles
AC_CONFIG_FILES([Makefile src/Makefile test/Makefile])

AC_OUTPUT
