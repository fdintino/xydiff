#ifndef XYLATINSTR_HXX__
#define XYLATINSTR_HXX__

/* Class that handles transcode value of XMLString */

#include "xercesc/dom/deprecated/DOMString.hpp"
#include "XyDiff/include/XyStr.hpp"
#include <stdio.h>
#include <iostream>


/* Gregory - Nov 2003: Nov support null characters inside strings, in that case the string length must be specified */

class XyLatinStr : public XyStr {
	public:
		XyLatinStr(const XMLCh* const toTranscode, int size=-1, const int fastOp=NO_FAST_OPTION, bool escapeSequenceXyHack=false);
		XyLatinStr(const char * const toTranscode, int size=-1, const int fastOp=NO_FAST_OPTION);

		static char * CreateFromUTF8(const char * toTranscode, int size=-1, bool escapeSequenceXyHack=false);
		static void ConvertFromUTF8(std::string & inoutStr, bool escapeSequenceXyHack=false);
		
		const char*  localForm();
		const XMLCh* wideForm();
	private :
		// copy is not allowed
		XyLatinStr& operator=(const XyLatinStr&);
		XyLatinStr(const XyLatinStr&);
		bool theEscapeSequenceXyHack;
	} ;

std::ostream& operator<<(std::ostream& target, XyLatinStr& toDump);

#endif
