#ifndef XYINT_HXX__
#define XYINT_HXX__

#include <iostream>
#include "xercesc/util/XMLString.hpp"


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
