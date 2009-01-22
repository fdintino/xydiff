#include "include/XyInt.hpp"
#include "include/XyLatinStr.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"

#include <stdlib.h>

/*
 * XyInt functions (analog to XyStr)
 */

XyInt::XyInt(const XMLCh* str, int size) : theIntValue(0) {
	if ((str==NULL)||(size==0)) return;
	if (size<0) size = xercesc_3_0::XMLString::stringLen(str);

	XMLCh *tmp = new XMLCh[size+1];
	memcpy(tmp, str, size*sizeof(XMLCh));
	tmp[size]=xercesc_3_0::chNull;
	
	xercesc_3_0::XMLString::trim(tmp);
	
	theIntValue  = xercesc_3_0::XMLString::parseInt(tmp);
	delete [] tmp;
}

XyInt::XyInt(const char* str, int size) {
	if ((str==NULL)||(size==0)) return;
	if (size<0) size = strlen(str);
	
	char *tmp = new char[size+1];
	memcpy(tmp, str, size*sizeof(char));
	tmp[size]='\0';
	
	xercesc_3_0::XMLString::trim(tmp);
	
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

