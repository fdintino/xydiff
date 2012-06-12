#ifndef __VM2_TOOLS_HXX_GFBFD__
#define __VM2_TOOLS_HXX_GFBFD__

#include "xercesc/util/XMLString.hpp"
#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMDocument.hpp"

#include "xydiff/VersionManagerException.hpp"

#include <string>
#include <stdio.h>


std::ostream& operator << (std::ostream& target, const VersionManagerException &e) ;

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
int intmin(int x, int y);
int intmax(int x, int y);

#if defined(_WIN32) || defined(_WIN64)
#ifdef NATIVEDLL_EXPORTS
#define NATIVEDLL_API extern "C" __declspec(dllexport)
#else
#define NATIVEDLL_API __declspec(dllimport)
#endif
#endif

#endif
