#ifndef __VM2_TOOLS_HXX_GFBFD__
#define __VM2_TOOLS_HXX_GFBFD__

#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMDocument.hpp"

#include <string>
#include <stdio.h>
#include <algorithm>

class VersionManagerException {
	public:
		VersionManagerException(const std::string &IncStatus,	const std::string &IncContext, const std::string &IncMessage);
		VersionManagerException(const std::string &IncContext, const std::string &IncMessage);
		std::string status ;
		std::string context ;
		std::string message ;
	} ;

// #define VERBOSE
//#define DONT_SAVE_RESULT
//#define DISABLE_POSITION_MOVING
//#define DISABLE_PRIORITY_FIFO
#define HW_PROF

// PentiumIII 600 MHz:
#define CLOCKRATE 602269556

#ifdef VERBOSE
#define vddprintf(a) printf a
#else
#define vddprintf(a)
#endif

extern FILE *timeFile ;

std::ostream& operator << (std::ostream& target, const VersionManagerException &e) ;

/*
 * get the child position of a node in the source document
 *
 * Valid values are 1...n, where n is the number of childs
 *
 */

int getPosition(xercesc::DOMNode *parent, xercesc::DOMNode *child) ;

/* Test if document is a delta */

bool isDelta(const xercesc::DOMDocument *doc) ;

/* 
 * it transforms a DOMString into an int; if it's error returns 0 
 * problem: how to know if 0 means error code or a valid value ???
 */

int watoi(const XMLCh* str);

const XMLCh * witoa(int intValue);

/*
 * tells if a certain file exists or not 
 */

bool existsFile(const char *fileName);
 
/*
 * Converts int to std::string
 */
std::string itoa (int n);

#endif
