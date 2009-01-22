#ifndef XYLEME_DOMPRINT_HXX
#define XYLEME_DOMPRINT_HXX

//#include "xercesc/dom/DOMString.hpp"
#include "xercesc/dom/DOMNode.hpp"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

//void          outputContent(ostream& target, const xercesc_3_0::DOMString &s);
void          outputContent(std::ostream& target, const XMLCh *s);
void          usage();
//ostream& operator<<(ostream& target, const xercesc_3_0::DOMString& toWrite);
std::ostream& operator<<(std::ostream& target, xercesc_3_0::DOMNode& toWrite);

#endif
