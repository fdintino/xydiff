/*
 *  StringDiff.cpp
 *  xydiff
 *
 *  Created by Frankie Dintino on 2/3/09.
 *
 */

#include "xercesc/util/PlatformUtils.hpp"


#include "Tools.hpp"
#include "DeltaException.hpp"
#include "include/XyLatinStr.hpp"
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
#include "include/XID_DOMDocument.hpp"


#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "include/XyStrDiff.hpp"



/*
 * XyStrDiff functions (character-by-character string diffs)
 */

XERCES_CPP_NAMESPACE_USE 

static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

XyStrDiff::XyStrDiff(DOMDocument *myDoc, DOMElement *elem, const char* strX, const char *strY, int sizeXStr, int sizeYStr)
{	
	doc = myDoc;
	root = elem;
	sizex = sizeXStr;
	sizey = sizeYStr;
	
	if ((strX==NULL)||(sizex==0)) return;
	if (sizex<0) sizex = strlen(strX);
	x = new char[sizex+1];
	memcpy(x, strX, sizex*sizeof(char));
	x[sizex]='\0';
	
	if ((strY==NULL)||(sizey==0)) return;
	if (sizey<0) sizey = strlen(strY);
	y = new char[sizey+1];
	memcpy(y, strY, sizey*sizeof(char));
	y[sizey]='\0';
	
	n = sizex;
	m = sizey;

	int malloclen = (sizeof(int))*(sizex+1)*(sizey+1);
	// c = LCS Length matrix
	c = (int*) malloc(malloclen);
	// d = Levenshtein Distance matrix
	d = (int*) malloc(malloclen);
	t = (int*) malloc(malloclen);
	
	currop = -1;
}

/*
 * Destructor
 */

XyStrDiff::~XyStrDiff(void)
{
	free(t);
	free(c);
	free(d);
	delete [] x;
	delete [] y;
}

void XyStrDiff::LevenshteinDistance()
{
	// Step 1
	int k, i, j, cost, distance;
	n = strlen(x);
	m = strlen(y);

	if (n != 0 && m != 0) {
		m++;
		n++;
		// Step 2
		for(k = 0; k < n; k++) {
			c[k] = 0;
			d[k] = k;
		}
		for(k = 0; k < m; k++) {
			c[k*n] = 0;
			d[k*n] = k;
		}
		
		// Step 3 and 4	
		for(i = 1; i < n; i++) {
			for(j = 1; j < m; j++) {
				// Step 5
				if (x[i-1] == y[j-1]) {
					cost = 0;
					c[j*n+i] = c[(j-1)*n + i-1] + 1;
				} else {
					cost = 1;
					c[j*n+i] = max(c[(j-1)*n + i], c[j*n + i-1]);
				}
				// Step 6
				int del = d[j*n+i-1] + 1;
				int ins = d[(j-1)*n+i] + 1;
				int sub = d[(j-1)*n+i-1] + cost;
				if (sub <= del && sub <= ins) {
					d[j*n+i] = sub;
					t[j*n+i] = STRDIFF_SUB;
				} else if (del <= ins) {
					d[j*n+i] = del;
					t[j*n+i] = STRDIFF_DEL;
				} else {
					d[j*n+i] = ins;
					t[j*n+i] = STRDIFF_INS;
				}
			}
		}
		distance = d[n*m-1];
		this->calculatePath();
		this->flushBuffers();
		
		vddprintf(("debugstr=%s\n", debugstr.c_str()));
	}
}

void XyStrDiff::calculatePath(int i, int j)
{
	if (i == -1) i = sizex;
	if (j == -1) j = sizey;
	if (i > 0 && j > 0 && (x[i-1] == y[j-1])) {
		this->calculatePath(i-1, j-1);
		this->registerBuffer(i-1, STRDIFF_NOOP, x[i-1]);
	} else {
		if (j > 0 && (i == 0 || c[(j-1)*n+i] >= c[j*n+i-1])) {
			this->calculatePath(i, j-1);
			this->registerBuffer(i, STRDIFF_INS, y[j-1]);
		} else if (i > 0 && (j == 0 || c[(j-1)*n+i] < c[j*n+i-1])) {
			this->calculatePath(i-1, j);
			this->registerBuffer(i-1, STRDIFF_DEL, x[i-1]);
		}
	}
}

void XyStrDiff::registerBuffer(int i, int optype, char chr)
{
	if (wordbuf.empty()) {
		wordbuf = chr;
	}
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

void XyStrDiff::flushBuffers()
{
	int startpos, len;
	if (currop == STRDIFF_NOOP) {
		return;
	} else if (currop == STRDIFF_SUB) {
		len = delbuf.length();
		startpos = xpos - len;
		debugstr.append("<tr pos=\"" + itoa(startpos) + "\" len=\"" + itoa(len) + "\">" + insbuf + "</r>\n");
		
		try {
			DOMElement *r = doc->createElement(XMLString::transcode("tr"));
			r->setAttribute(XMLString::transcode("pos"), XMLString::transcode((itoa(startpos)).c_str()));
			r->setAttribute(XMLString::transcode("len"), XMLString::transcode((itoa(len)).c_str()));		
			DOMText *textNode = doc->createTextNode(XMLString::transcode(insbuf.c_str()));
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
		
		delbuf = "";
		insbuf = "";
	} else if (currop == STRDIFF_INS) {
		startpos = xpos;
		debugstr.append("<ti pos=\"" + itoa(startpos) + "\">"+insbuf+"</i>\n");
		
		try {
			DOMElement *r = doc->createElement(XMLString::transcode("ti"));
			r->setAttribute(XMLString::transcode("pos"), XMLString::transcode((itoa(startpos)).c_str()));
			DOMText *textNode = doc->createTextNode(XMLString::transcode(insbuf.c_str()));
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
		
		
		insbuf = "";
	} else if (currop == STRDIFF_DEL) {
		len = delbuf.length();
		startpos = xpos - len;
		debugstr.append("<td pos=\"" + itoa(startpos) + "\" len=\"" + itoa(len) + "\" />\n");
		try {
			DOMElement *r = doc->createElement(XMLString::transcode("td"));
			r->setAttribute(XMLString::transcode("pos"), XMLString::transcode((itoa(startpos)).c_str()));
			r->setAttribute(XMLString::transcode("len"), XMLString::transcode((itoa(len)).c_str()));		
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
		delbuf = "";
	}
}

