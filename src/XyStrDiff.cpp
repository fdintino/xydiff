/*
 *  StringDiff.cpp
 *  xydiff
 *
 *  Created by Frankie Dintino on 2/3/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "include/XyStrDiff.hpp"

#include "include/XyLatinStr.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"

#include <stdlib.h>
#include <string.h>
#include <sstream>

/*
 * XyStrDiff functions (character-by-character string diffs)
 */

XERCES_CPP_NAMESPACE_USE 

#define STRDIFF_NOOP 0
#define STRDIFF_SUB 1
#define STRDIFF_INS 2
#define STRDIFF_DEL 3

XyStrDiff::XyStrDiff(const char* strX, const char *strY, int sizeXStr, int sizeYStr)
{	
	
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

int XyStrDiff::LevenshteinDistance()
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
		this->getPath();
		std::cout << s1 << std::endl;
		std::cout << s2 << std::endl;
		this->flushBuffers();
		std::cout << "outstr: " << outstr << std::endl;
		return distance;
	} else  {
		return -1; //a negative return value means that one or both strings are empty.
	}
}

void XyStrDiff::getPath(int i, int j)
{
	if (i == -1) i = sizex;
	if (j == -1) j = sizey;
	
	if (i > 0 && j > 0 && (x[i-1] == y[j-1])) {
		this->getPath(i-1, j-1);
		s1.append(" ");
		s2 += x[i-1];
		this->alterText(i, j, STRDIFF_NOOP, x[i-1]);
	} else {
		if (j > 0 && (i == 0 || c[(j-1)*n+i] >= c[j*n+i-1])) {
			this->getPath(i, j-1);
			s1.append("+");
			s2 += y[j-1];
			this->alterText(i, j, STRDIFF_INS, y[j-1]);
		} else if (i > 0 && (j == 0 || c[(j-1)*n+i] < c[j*n+i-1])) {
			this->getPath(i-1, j);
			s1.append("-");
			s2 += x[i-1];
			this->alterText(i, j, STRDIFF_DEL, x[i-1]);
		}
	}
}



void XyStrDiff::alterText(int i, int j, int optype, char chr)
{
	if (wordbuf.empty()) {
		wordbuf = chr;
	}
	xpos = i;
	ypos = j;
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
			noopbuf += chr;
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
		noopbuf += chr;
		this->flushBuffers();
		currop = optype;
	}
}



void XyStrDiff::flushBuffers()
{
	if (currop == STRDIFF_NOOP) {
		outstr.append(noopbuf);
		noopbuf = "";
	} else if (currop == STRDIFF_SUB) {
		int start = xpos - insbuf.length();
		outstr.append("<r st=\"" + itoa(start) + "\" len=\"" + itoa(delbuf.length()) + " repl=\"" + insbuf + "\"\"/>");
		delbuf = "";
		insbuf = "";
	} else if (currop == STRDIFF_INS) {
		outstr.append("<i>"+insbuf+"</i>");
		insbuf = "";
	} else if (currop == STRDIFF_DEL) {
		outstr.append("<d>"+delbuf+"</d>");
		delbuf = "";
	}
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