#ifndef XyUTF8STR_HXX__
#define XyUTF8STR_HXX__

/* Class that handles transcode value of XMLString */

#include "xercesc/dom/deprecated/DOMString.hpp"
#include "XyDiff/include/XyStr.hpp"

#include <stdio.h>
#include <iostream>

class XyUTF8Str : public XyStr {
	public:
		XyUTF8Str(const XMLCh* const toTranscode, int size=-1, const int fastOp=NO_FAST_OPTION);
		XyUTF8Str(const char * const toTranscode, int size=-1, const int fastOp=NO_FAST_OPTION);

		const char*  localForm() ;
		const XMLCh* wideForm() ;

	private :
		// copy is not allowed
		XyUTF8Str& operator=(const XyUTF8Str&);
		XyUTF8Str(const XyUTF8Str&);
		
	} ;

std::ostream& operator<<(std::ostream& target, XyUTF8Str& toDump);

#endif
