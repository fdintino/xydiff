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

/*****************************************************/
/*Function prototypes and libraries needed to compile*/
/*****************************************************/


int levenshtein_distance(char *s,char*t);
int minimum(int a,int b,int c);

/*****************************************************/
/*                     XyStrDiff                     */
/*****************************************************/

class XyStrDiff {
public:
	XyStrDiff(const char* x, const char *y, int sizex=-1, int sizey=-1);
	~XyStrDiff();
	int LevenshteinDistance();
private :
	char *x;
	char *y;
	int *c;
	int *d;
	char *t;
	int sizex, sizey;
};

//std::ostream& operator<<(std::ostream& target, const XyStrDiff& toDump);

template <class T> const T& max ( const T& a, const T& b ) {
	return (b<a)?a:b;     // or: return comp(b,a)?a:b; for the comp version
}

#endif
