#ifndef XYLATINDOC_HXX__
#define XYLATINDOC_HXX__

/* Class that handles transcode value of XMLString */

#include "xercesc/dom/deprecated/DOMString.hpp"
#include "XyDiff/include/XyStr.hpp"
#include <stdio.h>
#include <iostream>


/* Gregory - Nov 2003: Nov support null characters inside strings, in that case the string length must be specified */

class XyLatinDocument : public XyStr {
	public:
		XyLatinDocument(const XMLCh* const toTranscode, int size=-1, const int fastOp=NO_FAST_OPTION);
		XyLatinDocument(const char * const toTranscode, int size=-1, const int fastOp=NO_FAST_OPTION);

		const char*  localForm();
		const XMLCh* wideForm();
	private :
		// copy is not allowed
		XyLatinDocument& operator=(const XyLatinDocument&);
		XyLatinDocument(const XyLatinDocument&);
	} ;

#endif
