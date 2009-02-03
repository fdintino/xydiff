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

/*****************************************************/
/*                     XyStrDiff                     */
/*****************************************************/

class XyStrDiff {
public:
	XyStrDiff(const char* x, const char *y, int sizex=-1, int sizey=-1);
	~XyStrDiff();
	int LevenshteinDistance();
	void getPath(int i=-1, int j=-1);
	void XyStrDiff::alterText(int i, int j, int optype, char chr);
	void XyStrDiff::flushBuffers();
private :
	int xpos, ypos;
	char *x;
	char *y;
	int *c;
	int *d;
	int *t;
	int currop; // Current operation in alterText()
	std::string wordbuf, subbuf, insbuf, delbuf, outstr;
	std::string s1;
	std::string s2;
	int sizex, sizey, m, n;
};

//std::ostream& operator<<(std::ostream& target, const XyStrDiff& toDump);

template <class T> const T& max ( const T& a, const T& b ) {
	return (b<a)?a:b;     // or: return comp(b,a)?a:b; for the comp version
}

std::string itoa (int n);

#endif
