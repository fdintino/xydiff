#ifndef XYINT_HXX__
#define XYINT_HXX__

/* Class to transform directly a DOMString into an int */

#include "include/XyStr.hpp"

// #include "xercesc/dom/deprecated/DOMString.hpp"
#include <stdio.h>
#include <iostream>

class XyInt {
	public:
		XyInt(const XMLCh* str, XMLSize_t size=0);
		XyInt(const char* str, size_t size=0);
		int getValue() const ;
		operator int() const ;
	private :
		int theIntValue;
		bool theValid;		
	} ;

std::ostream& operator<<(std::ostream& target, const XyInt& toDump);

class XySize {
	public:
		XySize(const XMLCh* str, XMLSize_t size=0);
		XySize(const char* str, size_t size=0);
		XMLSize_t parseInt(const XMLCh* const toConvert);
		XMLSize_t getValue() const ;
		operator XMLSize_t() const ;
	private :
		XMLSize_t theSizeValue;
		bool theValid;		
	} ;

std::ostream& operator<<(std::ostream& target, const XySize& toDump);

#endif
