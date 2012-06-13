#ifndef XYLEME_DOMPRINT_HXX
#define XYLEME_DOMPRINT_HXX

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "xercesc/dom/DOMNode.hpp"

void          outputContent(std::ostream& target, const XMLCh *s);
void          usage();
std::ostream& operator<<(std::ostream& target, xercesc::DOMNode& toWrite);

#endif
