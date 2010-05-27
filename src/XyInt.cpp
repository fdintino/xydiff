#include "include/XyInt.hpp"
#include "include/XyLatinStr.hpp"
#include "DeltaException.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"
#include "xercesc/util/Janitor.hpp"

#include <stdlib.h>

/*
 * XyInt functions (analog to XyStr)
 */

XERCES_CPP_NAMESPACE_USE 

XyInt::XyInt(const XMLCh* str, int size) : theIntValue(0) {
	if ((str==NULL)||(size==0)) return;
	if (size<0) size = XMLString::stringLen(str);

	XMLCh *tmp = new XMLCh[size+1];
	memcpy(tmp, str, size*sizeof(XMLCh));
	tmp[size]=chNull;
	
	XMLString::trim(tmp);
	
	theIntValue  = XMLString::parseInt(tmp);
	delete [] tmp;
}

XyInt::XyInt(const char* str, int size) {
	if ((str==NULL)||(size==0)) return;
	if (size<0) size = strlen(str);
	
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

XMLSize_t XySize::parseInt(const XMLCh* const toConvert)
{
    // If no string, or empty string, then it is a failure
    if ((!toConvert) || (!*toConvert))
        THROW_AWAY(("Null pointer"));

	XMLCh* trimmedStr = XMLString::replicate(toConvert);
	ArrayJanitor<XMLCh> jan1(trimmedStr);
	XMLString::trim(trimmedStr);
    XMLSize_t trimmedStrLen = XMLString::stringLen(trimmedStr);

	if ( !trimmedStrLen )
		THROW_AWAY(("Trimmed string is null pointer"));

	char *nptr = XMLString::transcode(trimmedStr);
    ArrayJanitor<char> jan2(nptr);

    char *endptr;
    long retVal = strtol(nptr, &endptr, 10);

	// check if all chars are valid char
	if ( (endptr - nptr) != (int) trimmedStrLen) {
		THROW_AWAY(("Invalid characters"));
	}

	return (XMLSize_t) retVal;
}

XySize::XySize(const XMLCh* str, XMLSize_t size) : theSizeValue(0) {
	if ((str==NULL)) return;
	if (size==0) size = XMLString::stringLen(str);
	if (size==0) return;

	XMLCh *tmp = new XMLCh[size+1];
	memcpy(tmp, str, size*sizeof(XMLCh));
	tmp[size]=chNull;
	
	XMLString::trim(tmp);
	
	theSizeValue  = XySize::parseInt(tmp);
	delete [] tmp;
}

XySize::XySize(const char* str, size_t size) {
	if ((str==NULL)||(size==0)) return;
	if (size<0) size = strlen(str);
	
	char *tmp = new char[size+1];
	memcpy(tmp, str, size*sizeof(char));
	tmp[size]='\0';
	
	XMLString::trim(tmp);

	XMLSize_t trimmedStrLen = XMLString::stringLen(tmp);

	if ( !trimmedStrLen )
		THROW_AWAY(("Trimmed string is null pointer"));

    ArrayJanitor<char> jan2(tmp);

    char *endptr;
    long retVal = strtol(tmp, &endptr, 10);

	// check if all chars are valid char
	if ( (endptr - tmp) != (int) trimmedStrLen) {
		THROW_AWAY(("Invalid characters"));
	}

	theSizeValue  = (XMLSize_t) retVal;
	delete [] tmp;
}

XMLSize_t XySize::getValue() const {
	return theSizeValue;
}

XySize::operator XMLSize_t() const {
	return this->getValue();
}

std::ostream& operator<<(std::ostream& target, const XySize& toDump)
{
    target << toDump.getValue();
    return target;
}

