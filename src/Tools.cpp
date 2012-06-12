#include "Tools.hpp"

#include <iostream>

#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"

#include "xydiff/XyLatinStr.hpp"

XERCES_CPP_NAMESPACE_USE


//--------------------------------------------------------------------------
//
//       Version Manager Exception
//
//--------------------------------------------------------------------------

VersionManagerException::VersionManagerException(const std::string &IncStatus, const std::string &IncContext, const std::string &IncMessage) {
  status = IncStatus ;
	context = IncContext ;
	message = IncMessage ;
	}

VersionManagerException::VersionManagerException(const std::string &IncContext, const std::string &IncMessage) {
  status = "Runtime Error" ;
	context = IncContext ;
	message = IncMessage ;
	}

std::ostream &operator << (std::ostream& target, const VersionManagerException &e) {
  target << "\n\n *** *** Version Manager Exception" << std::endl ;
	target << "  -Status   : " << e.status << std::endl ;
	target << "  -Context  : " << e.context << std::endl ;
	target << "  -Messages : " << e.message << std::endl ;
	return target ;
	}

/* 
 * it transforms a DOMString into an int; if it's error returns 0 
 * problem: how to know if 0 means error code or a valid value ???
 */

int watoi(const XMLCh* str) {
	int intValue = atoi(XyLatinStr(str).localForm());
	// cout << "Tools.cxx -> watoi() : intValue=" << intValue << endl;
	return intValue;
}

const XMLCh * witoa(int intValue)
{
	const XMLCh *str = XMLString::transcode( itoa(intValue).c_str() );
	return str;
}

/*
 * tells if a certain file exists or not 
 */

bool existsFile(const char *fileName) {
	FILE *testFile = fopen(fileName, "r");
	if ( testFile != NULL ) {
		fclose(testFile);
	}
	return (testFile != NULL);
}

std::string itoa (int n)
{	
	char * s = new char[17];
	std::string u;
	
	if (n < 0) {         //turns n positive
		n = (-1 * n); 
		u = "-";         //adds '-' on result string
	}
	
	int i = 0;  // s counter
	
	do {
		s[i++] = n % 10 + '0'; //conversion of each digit of n to char
		n -= n % 10;           //update n value
	} while ((n /= 10) > 0);
	
	for (int j = i-1; j >= 0; j--) { 
		u += s[j];    //building our string number
	}
	
	delete[] s;       //free-up the memory!
	return u;
}

int intmin(int x, int y)
{
	return (x < y) ? x : y;
}

int intmax(int x, int y)
{
	return (x > y) ? x : y;
}