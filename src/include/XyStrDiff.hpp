/*
 *  StringDiff.h
 *  xydiff
 *
 *  Created by Frankie Dintino on 2/3/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef XyStrDiff_HXX__
#define XyStrDiff_HXX__

/* Class to do diffs on strings character-by-character */

#include "include/XyStr.hpp"
#include "include/XID_DOMDocument.hpp"

// #include "xercesc/dom/deprecated/DOMString.hpp"
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMNodeFilter.hpp"


#define STRDIFF_NOOP 0
#define STRDIFF_SUB 1
#define STRDIFF_INS 2
#define STRDIFF_DEL 3

/*****************************************************/
/*                     XyStrDiff                     */
/*****************************************************/

class XyStrDiff {
public:
	XyStrDiff(xercesc::DOMDocument *myDoc, xercesc::DOMElement *elem, const XMLCh *x, const XMLCh *y, XMLSize_t sizex=0, XMLSize_t sizey=0);
	~XyStrDiff();
	void LevenshteinDistance();
	void calculatePath(XMLSize_t i, XMLSize_t j);
	void registerBuffer(XMLSize_t i, int optype, XMLCh chr);
	void flushBuffers();
private :
	xercesc::DOMImplementation* impl;
	xercesc::DOMDocument *doc;
	xercesc::DOMElement *root;
	XMLSize_t xpos, ypos;
	XMLCh *x;
	XMLCh *y;
	XMLSize_t *c;
	XMLSize_t *d;
	XMLSize_t *t;
	int currop; // Current operation in alterText()
  std::basic_string<XMLCh> insbuf;
  std::basic_string<XMLCh> delbuf;
  XMLSize_t sizex, sizey;
  XMLSize_t m, n;
};


#endif
