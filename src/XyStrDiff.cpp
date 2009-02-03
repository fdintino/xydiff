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

/*
 * XyStrDiff functions (character-by-character string diffs)
 */

XERCES_CPP_NAMESPACE_USE 

XyStrDiff::XyStrDiff(const char* strX, const char *strY, int sizeXStr, int sizeYStr) {
	
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
	
	// c = LCS Length matrix
	c = (int*) malloc((sizeof(int))*(sizex+1)*(sizey+1));
	// d = Levenshtein Distance matrix
	d = (int*) malloc((sizeof(int))*(sizex+1)*(sizey+1));
	t = (char*) malloc((sizeof(char))*(sizex+1)*(sizey+1));
}

/*
 * Destructor
 */

XyStrDiff::~XyStrDiff(void) {
	free(t);
	free(c);
	free(d);
}

int XyStrDiff::LevenshteinDistance() {
	// Step 1
	int k, i, j, n, m, cost, distance;
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
					t[j*n+i] = 'S';
				} else if (del <= ins) {
					d[j*n+i] = del;
					t[j*n+i] = 'D';
				} else {
					d[j*n+i] = ins;
					t[j*n+i] = 'I';
				}
			}
		}
		distance = d[n*m-1];

		return distance;
	} else  {
		return -1; //a negative return value means that one or both strings are empty.
	}
	
}

void getPath(int *c, char *x, char *y, int i, int j, char *t);
void getPath(int *c, char *x, char *y, int i, int j, char *t) {
	
}