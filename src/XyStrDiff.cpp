/*
 *  StringDiff.cpp
 *  xydiff
 *
 *  Created by Frankie Dintino on 2/3/09.
 *
 */

#include "xercesc/util/PlatformUtils.hpp"


#include "Tools.hpp"
#include "xydiff/DeltaException.hpp"
#include "xydiff/XyLatinStr.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"

#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/dom/DOMImplementationLS.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"
#include "xercesc/dom/DOMException.hpp"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMText.hpp"
#include "xercesc/dom/DOMTreeWalker.hpp"

#include "xercesc/dom/DOMNodeList.hpp"

#include "xercesc/dom/DOMAttr.hpp"
#include "xercesc/util/XMLUniDefs.hpp"
#include "xercesc/sax/ErrorHandler.hpp"
#include "xercesc/sax/SAXException.hpp"
#include "xercesc/sax/SAXParseException.hpp"
#include "xydiff/XID_DOMDocument.hpp"


#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "xydiff/XyStrDiff.hpp"



/*
 * XyStrDiff functions (character-by-character string diffs)
 */

XERCES_CPP_NAMESPACE_USE 

static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

XyStrDiff::XyStrDiff(DOMDocument *myDoc, DOMElement *elem, const XMLCh *strX, const XMLCh *strY, int sizeXStr, int sizeYStr)
{	
	doc = myDoc;
	root = elem;
	sizex = sizeXStr;
	sizey = sizeYStr;
	
	if ((strX==NULL)||(sizex==0)) return;
	if (sizex<0) sizex = XMLString::stringLen(strX);
	if ((strY==NULL)||(sizey==0)) return;
	if (sizey<0) sizey = XMLString::stringLen(strY);

	x = XMLString::replicate(strX);
	y = XMLString::replicate(strY);

	n = sizex;
	m = sizey;
	c = NULL;

	currop = -1;
}

/*
 * Destructor
 */

XyStrDiff::~XyStrDiff(void)
{
	if (c != NULL) {
		free(c);
	}
	XMLString::release(&x);
	XMLString::release(&y);
}

void XyStrDiff::LevenshteinDistance()
{
	if (n > 65535 || m > 65535) {
		// do a simple replace
		this->simpleReplace();
		return;
	}

	if (c == NULL) {
		int cmalloclen = (sizeof(unsigned short))*(sizex+1)*(sizey+1);
		c = (unsigned short*) malloc(cmalloclen);
	}

	// Step 1
	unsigned short k, i, j, cost, distance;
	
	n = XMLString::stringLen(x);
	m = XMLString::stringLen(y);

	if (n != 0 && m != 0) {
		m++;
		n++;
		// Step 2
		for(k = 0; k < n; k++) {
			c[k] = 0;
		}
		for(k = 0; k < m; k++) {
			c[k*n] = 0;
		}
		int del, ins, sub, a, b;
		// Step 3 and 4	
		for(i = 1; i < n; i++) {
			for(j = 1; j < m; j++) {
				// Step 5
				if (x[i-1] == y[j-1]) {
					cost = 0;
					c[j*n+i] = c[(j-1)*n + i-1] + 1;
				} else {
					cost = 1;
					a = c[(j-1)*n + i];
					b = c[j*n + i-1];
					c[j*n+i] = (a > b) ? a : b;
				}
			}
		}
    this->calculatePath();
    this->flushBuffers();
	}
}

void XyStrDiff::calculatePath(int i, int j)
{
	int malloclen = (sizeof(int))*4*(sizex + 1 + sizey+1);

	// ops is a 2-dimensional integer array with 4 values per row
	// 0 - xpos, the first parameter passed to registerBuffer
	// 1 - the operation (integer constant, STRDIFF_NOOP, STRDIFF_INS, or STRDIFF_DEL)
	// 2 - whether to use x or y as the string from which we are passing the character
	//      1 = x, 2 = y
	// 3 - the character position within the string determined by (2) that is passed to registerBuffer()

	int *ops = (int *) malloc(malloclen);

	if (i == -1) i = sizex;
	if (j == -1) j = sizey;

	int opnum = 0;
	while (i >= 0 && j >= 0) {
		if (i > 0 && j > 0 && (x[i-1] == y[j-1])) {
			ops[opnum*4 + 0] = i-1;
			ops[opnum*4 + 1] = STRDIFF_NOOP;
			ops[opnum*4 + 2] = 1;
			ops[opnum*4 + 3] = i-1;
			i = i-1;
			j = j-1;
			opnum++;
			continue;				
		} else {
			if (j > 0 && (i == 0 || c[(j-1)*n+i] >= c[j*n+i-1])) {
				ops[opnum*4 + 0] = i;
				ops[opnum*4 + 1] = STRDIFF_INS;
				ops[opnum*4 + 2] = 2;
				ops[opnum*4 + 3] = j-1;
				j = j-1;
				opnum++;
				continue;
			} else if (i > 0 && (j == 0 || c[(j-1)*n+i] < c[j*n+i-1])) {
				if (i == sizex && j == sizey) {
					ops[opnum*4 + 0] = i;
				} else {
					ops[opnum*4 + 0] = i-1;
				}
				ops[opnum*4 + 1] = STRDIFF_DEL;
				ops[opnum*4 + 2] = 1;
				ops[opnum*4 + 3] = i-1;
				i = i - 1;
				opnum++;
				continue;
			}
		}
		break;
	}
	int xpos;
	int optype;
	int charpos;
	int z;
	
	for (z = opnum-1; z >= 0; z--) {
		xpos    = ops[z*4 + 0];
		optype  = ops[z*4 + 1];
		charpos = ops[z*4 + 3];
		if (ops[z*4 + 2] == 1) {
			this->registerBuffer(xpos, optype, x[charpos]);
		} else {
			this->registerBuffer(xpos, optype, y[charpos]);
		}
	}
}

void XyStrDiff::registerBuffer(int i, int optype, XMLCh chr)
{

	xpos = i;

	if (currop == -1) {
		currop = optype;

	} 
	if (currop == STRDIFF_SUB) {
		if (optype == STRDIFF_DEL) {
      delbuf += chr;
		} else if (optype == STRDIFF_INS) {
      insbuf += chr;
		} else {
			this->flushBuffers();
			currop = optype;
		}
	}
	else if (optype == STRDIFF_DEL) {
		currop = optype;
		delbuf += chr;
	}
	else if (optype == STRDIFF_INS) {
		currop = (currop == STRDIFF_DEL) ? STRDIFF_SUB : STRDIFF_INS;
		insbuf += chr;
	}
	else if (optype == STRDIFF_NOOP) {
		this->flushBuffers();
		currop = optype;
	}
}

void XyStrDiff::simpleReplace()
{
	int startpos, len;
	XMLCh tempStrA[100];
	XMLCh tempStrB[100];

	startpos = 0;
	len = sizex;
	try {
		XMLString::transcode("xy:tr", tempStrA, 99);
		DOMElement *r = doc->createElement(tempStrA);
		XMLString::transcode("pos", tempStrA, 99);
		XMLString::transcode(itoa(startpos).c_str(), tempStrB, 99);
		r->setAttribute(tempStrA, tempStrB);
		XMLString::transcode("len", tempStrA, 99);
		XMLString::transcode(itoa(len).c_str(), tempStrB, 99);
		r->setAttribute(tempStrA, tempStrB);

		DOMText *textNode = doc->createTextNode(y);

		r->appendChild((DOMNode *)textNode);
		root->appendChild((DOMNode *)r);
	}
	catch (const XMLException& toCatch) {
		std::cout << "XMLException: " << XMLString::transcode(toCatch.getMessage()) << std::endl;
	}
	catch (const DOMException& toCatch) {
		std::cout << "DOMException: " << XMLString::transcode(toCatch.getMessage()) << std::endl;
	}
	catch (...) {
		std::cout << "Unexpected Exception: " << std::endl;
	}
}


void XyStrDiff::flushBuffers()
{
	int startpos, len;
	XMLCh tempStrA[100];
	XMLCh tempStrB[100];
	if (currop == STRDIFF_NOOP) {
		return;
	} else if (currop == STRDIFF_SUB) {
    	len = delbuf.length();
		startpos = xpos - len;
		
		try {
			XMLString::transcode("xy:tr", tempStrA, 99);
			DOMElement *r = doc->createElement(tempStrA);
			XMLString::transcode("pos", tempStrA, 99);
			XMLString::transcode(itoa(startpos).c_str(), tempStrB, 99);
			r->setAttribute(tempStrA, tempStrB);
			XMLString::transcode("len", tempStrA, 99);
			XMLString::transcode(itoa(len).c_str(), tempStrB, 99);
			r->setAttribute(tempStrA, tempStrB);

			DOMText *textNode = doc->createTextNode(insbuf.c_str());

			r->appendChild((DOMNode *)textNode);
			root->appendChild((DOMNode *)r);
		}
		catch (const XMLException& toCatch) {
			std::cout << "XMLException: " << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (const DOMException& toCatch) {
			std::cout << "DOMException: " << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (...) {
			std::cout << "Unexpected Exception: " << std::endl;
		}

    delbuf.clear();
    insbuf.clear();
	} else if (currop == STRDIFF_INS) {
		startpos = xpos;
		
		try {
			XMLString::transcode("xy:ti", tempStrA, 99);
			DOMElement *r = doc->createElement(tempStrA);

			XMLString::transcode("pos", tempStrA, 99);
			XMLString::transcode(itoa(startpos).c_str(), tempStrB, 99);
			r->setAttribute(tempStrA, tempStrB);

			DOMText *textNode = doc->createTextNode(insbuf.c_str());

			r->appendChild((DOMNode *)textNode);
			root->appendChild((DOMNode *)r);
		}
		catch (const XMLException& toCatch) {
			std::cout << "Exception message is: \n" << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (const DOMException& toCatch) {
			std::cout << "Exception message is: \n" << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (...) {
			std::cout << "Unexpected Exception" << std::endl; 
		}

    insbuf.clear();
	} else if (currop == STRDIFF_DEL) {
    len = delbuf.length();
		startpos = xpos - len;
		try {
			XMLString::transcode("xy:td", tempStrA, 99);
			DOMElement *r = doc->createElement(tempStrA);
			XMLString::transcode("pos", tempStrA, 99);
			XMLString::transcode(itoa(startpos).c_str(), tempStrB, 99);
			r->setAttribute(tempStrA, tempStrB);
			XMLString::transcode("len", tempStrA, 99);
			XMLString::transcode(itoa(len).c_str(), tempStrB, 99);
			r->setAttribute(tempStrA, tempStrB);		
			root->appendChild((DOMNode *)r);
		}
		catch (const XMLException& toCatch) {
			std::cout << "Exception message is: \n" << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (const DOMException& toCatch) {
			std::cout << "Exception message is: \n" << XMLString::transcode(toCatch.getMessage()) << std::endl;
		}
		catch (...) {
			std::cout << "Unexpected Exception" << std::endl;
		}
    delbuf.clear();
	}
}

