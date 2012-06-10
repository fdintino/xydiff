#ifndef XYINT_HXX__
#define XYINT_HXX__

/* Class to transform directly a DOMString into an int */

#include "xydiff/XyStr.hpp"

// #include "xercesc/dom/deprecated/DOMString.hpp"
#include <stdio.h>
#include <iostream>

class XyInt {
	public:
		XyInt(const XMLCh* str, int size=-1);
		XyInt(const char* str, int size=-1);
		int getValue() const ;
		operator int() const ;
	private :
		int theIntValue;
		bool theValid;		
	} ;

std::ostream& operator<<(std::ostream& target, const XyInt& toDump);

#endif
