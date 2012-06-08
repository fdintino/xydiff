SUBDIRS = include

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/include
AM_LDFLAGS = -lxerces-c -L$(XERCESC_PREFIX)/lib

lib_LTLIBRARIES = libXyDelta.la

libXyDelta_la_SOURCES = convertUTF.cpp StringPusher.cpp \
                    ComputeDelta.cpp \
                    Diff_DeltaConstructor.cpp \
                    Diff_NodesManager.cpp \
                    Diff_UniqueIdHandler.cpp \
                    DeltaApply.cpp \
                    DeltaException.cpp \
                    DeltaManager.cpp \
                    DeltaReverse.cpp \
                    DeltaSortOperations.cpp \
                    easy_css.cpp \
                    lcss.cpp \
                    lookup2.cpp \
                    Tools.cpp \
                    XID_map.cpp \
                    XID_DOMDocument.cpp \
                    XyDeltaFileImpl.cpp \
                    XyDeltaDomImpl.cpp \
                    XyInt.cpp \
                    XyLatinStr.cpp \
                    XyLatinDocument.cpp \
                    XyStr.cpp \
                    XyStrDiff.cpp \
                    XyStrDelta.cpp \
                    XyUTF8Str.cpp \
                    XyUTF8Document.cpp

libXyDelta_la_CPPFLAGS = $(XERCESC_INCLUDE) -I$(top_srcdir)/src -I$(top_srcdir)/src/include
# libXyDelta_la_LIBADD = $(XERCESC_LIB)

bin_PROGRAMS = xydiff xydelta
xydiff_SOURCES = execComputeDelta.cpp
xydiff_CPPFLAGS = $(XERCESC_INCLUDE) -I$(top_srcdir)/src -I$(top_srcdir)/src/include
# xydiff_LIBADD = $(XERCESC_LIB)
xydiff_LDADD = libXyDelta.la

xydelta_SOURCES = execDeltaApply.cpp
xydelta_CPPFLAGS = $(XERCESC_INCLUDE) -I$(top_srcdir)/src -I$(top_srcdir)/src/include
xydelta_LDADD = libXyDelta.la
