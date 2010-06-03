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
	XyStrDiff(xercesc::DOMDocument *myDoc, xercesc::DOMElement *elem, const XMLCh *x, const XMLCh *y, int sizex=-1, int sizey=-1);
	~XyStrDiff();
	void LevenshteinDistance();
	void calculatePath(int i=-1, int j=-1);
	void registerBuffer(int i, int optype, XMLCh chr);
	void flushBuffers();
	void simpleReplace();
private :
	xercesc::DOMImplementation* impl;
	xercesc::DOMDocument *doc;
	xercesc::DOMElement *root;
	int xpos, ypos;
	XMLCh *x;
	XMLCh *y;
	uint16_t *c;
	unsigned short *d;
	char *t;
	int currop; // Current operation in alterText()
  std::basic_string<XMLCh> insbuf;
  std::basic_string<XMLCh> delbuf;
  int sizex, sizey;
  int m, n;
};


#endif
