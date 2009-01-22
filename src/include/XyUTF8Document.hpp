#ifndef XyUTF8DOC_HXX__
#define XyUTF8DOC_HXX__

/* Class that handles transcode value of XMLString */

// #include "xercesc/dom/deprecated/DOMString.hpp"
#include "include/XyStr.hpp"

#include <stdio.h>
#include <iostream>

class XyUTF8Document : public XyStr {
	public:
		XyUTF8Document(const XMLCh* const toTranscode, int size=-1, const int fastOp=NO_FAST_OPTION);
		XyUTF8Document(const char * const toTranscode, int size=-1, const int fastOp=NO_FAST_OPTION);

		const char*  localForm() ;
		const XMLCh* wideForm() ;

	private :
		// copy is not allowed
		XyUTF8Document& operator=(const XyUTF8Document&);
		XyUTF8Document(const XyUTF8Document&);
		
	} ;

#endif
