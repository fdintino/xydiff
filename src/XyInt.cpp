#include "include/XyInt.hpp"
#include "include/XyLatinStr.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"

#include <stdlib.h>

/*
 * XyInt functions (analog to XyStr)
 */

XERCES_CPP_NAMESPACE_USE 

XyInt::XyInt(const XMLCh* str, int size) : theIntValue(0) {
	if ((str==NULL)||size==0) return;
	if (size==-1) size = XMLString::stringLen(str);
	if (size == 0) return;
	XMLCh *tmp = new XMLCh[size+1];
	memcpy(tmp, str, size*sizeof(XMLCh));
	tmp[size]=chNull;
	
	XMLString::trim(tmp);
	
	theIntValue  = XMLString::parseInt(tmp);
	delete [] tmp;
}

XyInt::XyInt(const char* str, int size) {
	if ((str==NULL)||size==0) return;
	if (size==-1) size = strlen(str);
	if (size==0) return;
	char *tmp = new char[size+1];
	memcpy(tmp, str, size*sizeof(char));
	tmp[size]='\0';
	
	XMLString::trim(tmp);
	
	theIntValue  = atoi(tmp);
	delete [] tmp;
}

int XyInt::getValue() const {
	return theIntValue;
}

XyInt::operator int() const {
	return this->getValue();
}

std::ostream& operator<<(std::ostream& target, const XyInt& toDump)
{
    target << toDump.getValue();
    return target;
}

