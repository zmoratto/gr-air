#
# Copyright 2004, 2006, 2007 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

include $(top_srcdir)/Makefile.common

# Install this stuff so that it ends up as the gnuradio.vocoder module
# This usually ends up at:
#   ${prefix}/lib/python${python_version}/site-packages/gnuradio

ourpythondir = $(grpythondir)
ourlibdir    = $(grpyexecdir)

INCLUDES = $(STD_DEFINES_AND_INCLUDES) $(PYTHON_CPPFLAGS)

SWIGPYTHONARGS = -python $(SWIGPYTHONFLAGS) $(STD_DEFINES_AND_INCLUDES)


ALL_IFILES = 				\
	$(LOCAL_IFILES)			\
	$(NON_LOCAL_IFILES)

NON_LOCAL_IFILES =			\
	$(GNURADIO_I)


LOCAL_IFILES = 				\
	$(top_srcdir)/src/lib/air.i

# These files are built by SWIG.  The first is the C++ glue.
# The second is the python wrapper that loads the _howto shared library
# and knows how to call our extensions.

BUILT_SOURCES = \
	air.cc	\
	air.py

# This gets air.py installed in the right place
ourpython_PYTHON =	\
	air.py

ourlib_LTLIBRARIES = _air.la

# These are the source files that go into the shared library
_air_la_SOURCES = \
    air.cc \
    air_ms_pulse_detect.cc \
    air_ms_preamble.cc \
    air_ms_framer.cc \
    air_ms_ppm_decode.cc \
    air_ms_fmt_log.cc \
    air_ms_cvt_float.cc \
    air_ms_parity.cc \
    air_ms_ec_brute.cc \
    airi_ms_parity.cc \
    # Additional source modules here


# magic flags
_air_la_LDFLAGS = $(NO_UNDEFINED) -module -avoid-version

# link the library against the c++ standard library
_air_la_LIBADD = 	\
	$(PYTHON_LDFLAGS)	\
	$(GNURADIO_CORE_LA)	\
	-lstdc++

air.cc air.py: air.i $(ALL_IFILES)
	$(SWIG) $(SWIGPYTHONARGS) -module air -o air.cc $(LOCAL_IFILES)

# These headers get installed in ${prefix}/include/gnuradio
grinclude_HEADERS =			\
    air_ms_types.h \
    air_ms_pulse_detect.h \
    air_ms_preamble.h \
    air_ms_framer.h \
    air_ms_ppm_decode.h \
    air_ms_fmt_log.h \
    air_ms_cvt_float.h \
    air_ms_parity.h \
    air_ms_ec_brute.h \
    # Additional header files here

# These swig headers get installed in ${prefix}/include/gnuradio/swig
swiginclude_HEADERS = 			\
	$(LOCAL_IFILES)


MOSTLYCLEANFILES = $(BUILT_SOURCES) *.pyc *~ *.tmp

# Don't distribute output of swig
dist-hook:
	@for file in $(BUILT_SOURCES); do echo $(RM) $(distdir)/$$file; done
	@for file in $(BUILT_SOURCES); do $(RM) $(distdir)/$$file; done

