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

// #include "xercesc/dom/deprecated/DOMString.hpp"
#include <stdio.h>
#include <iostream>
#include <string>
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMElement.hpp"

#define STRDIFF_NOOP 0
#define STRDIFF_SUB 1
#define STRDIFF_INS 2
#define STRDIFF_DEL 3

/*****************************************************/
/*                     XyStrDiff                     */
/*****************************************************/

class XyStrDiff {
public:
	XyStrDiff(xercesc::DOMDocument *myDoc, xercesc::DOMElement *elem, const char* x, const char *y, int sizex=-1, int sizey=-1);
	~XyStrDiff();
	void LevenshteinDistance();
	void calculatePath(int i=-1, int j=-1);
	void XyStrDiff::registerBuffer(int i, int j, int optype, char chr);
	void XyStrDiff::flushBuffers();
private :
	xercesc::DOMImplementation* impl;
	xercesc::DOMDocument *doc;
	xercesc::DOMElement *root;
	int xpos, ypos;
	char *x;
	char *y;
	int *c;
	int *d;
	int *t;
	int currop; // Current operation in alterText()
	std::string wordbuf, subbuf, insbuf, delbuf, debugstr;
	int sizex, sizey, m, n;
};

//std::ostream& operator<<(std::ostream& target, const XyStrDiff& toDump);

template <class T> const T& max ( const T& a, const T& b ) {
	return (b<a)?a:b;     // or: return comp(b,a)?a:b; for the comp version
}

std::string itoa (int n);

#endif
