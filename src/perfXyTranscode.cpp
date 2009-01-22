#include "xercesc/util/XMLUniDefs.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/dom/DOMException.hpp"

#include "XyDiff/DeltaException.hpp"
#include "XyDiff/include/XID_DOMDocument.hpp"
#include "XyDiff/include/XID_map.hpp"
#include "XyDiff/Tools.hpp"
#include "XyDiff/include/XyStr.hpp"
#include "XyDiff/include/XyLatinStr.hpp"
#include "XyDiff/include/XyUTF8Str.hpp"
#include "XyDiff/include/XyUTF8Document.hpp"
#include "XyDiff/xyleme_DOMPrint.hpp"
#include <stdio.h>
#include <fstream>
#include <string>

#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/util/XMLString.hpp"

#include <stdarg.h>
#include <locale.h>
#include <time.h>

using namespace std;

void printTime(clock_t start, clock_t end, int nbOps, const char *name) {
	unsigned long x = end - start;
	float ms  = (float)x / (float)CLOCKS_PER_SEC;
        float inv = ((float)nbOps * (float)CLOCKS_PER_SEC) / (float)x;
	printf("each operations took %03f micro seconds\n", (1000000.0*ms)/(float)nbOps);
        printf("speed %s = %06f operation / s\n", name, inv);
}

int main(int argc, char **argv) {
  
	if (setlocale(LC_ALL, "fr_FR")) {
		printf("locale set to fr_FR\n");
	}
	else if (setlocale(LC_ALL, "en_US.ISO8859-15")) {
		printf("locale set to en_US.ISO8859-15\n");
	}
	try {
		xercesc_2_2::XMLPlatformUtils::Initialize();
	}
	catch(const xercesc_2_2::XMLException& toCatch) {
		cerr << "Error during Xerces-c Initialization.\n"
		     << "  Exception message:" << xercesc_2_2::XMLString::transcode(toCatch.getMessage()) << endl;
	}
	
	cout << "sizeof(char)=" << sizeof(char) << endl;
	cout << "sizeof(wchar_t)=" << sizeof(wchar_t) << endl;
	cout << "sizeof(XMLCh)=" << sizeof(XMLCh) << endl;

	std::string s1 = "test gregory";
	XyLatinStr::ConvertFromUTF8(s1);
	cout << "test 1: " << s1 << endl;

	std::string s2 = XyUTF8Str(L"test grégory").localForm();
	XyLatinStr::ConvertFromUTF8(s2);
	cout << "test 2: " << s2 << endl;
	
	const wchar_t * tmp = NULL;

	clock_t startT = clock();
	for(int i=0; i<1000000; i++) {
		XyLatinStr * x = new XyLatinStr("toto");
		tmp = x->wideForm();
                delete x;
	}
	clock_t endT = clock();
	printTime(startT, endT, 1000000, "clock()");

	startT = clock();
	for(int i=0; i<1000000; i++) {
		//const char * source = "toto fdfsdfgsdg gfgdfgfd gfgfd gfd fgdfg fgfd fdg dfgfd fdg dfg fg dfg dfg fg dfg fdg fg fdg dfg fdg dg ffgfgdfgbbbvbfddddddd er er ezzzzzzzzzzerzerzerer fdgfgdfg fdgfdgfg cbbvbbvbgdrfgdfgdf fdgfg";
		const char * source = "toto";
		bool latinOK = true;
		int max = strlen(source);
		for(int i=0;((i<max)&&latinOK);i++) if (source[i]<0) latinOK=false;
		char * res = new char[max+1];
		memcpy(res, source, max+1);
		delete [] res;
	}
	endT = clock();
	printTime(startT, endT, 1000000, "clock()");



	startT = clock();
	for(int i=0; i<1000000; i++) {
		XyLatinStr * x = new XyLatinStr("toto fdfsdfgsdg gfgdfgfd gfgfd gfd fgdfg fgfd fdg dfgfd fdg dfg fg dfg dfg fg dfg fdg fg fdg dfg fdg dg ffgfgdfgbbbvbfddddddd er er ezzzzzzzzzzerzerzerer fdgfgdfg fdgfdgfg cbbvbbvbgdrfgdfgdf fdgfg");
		tmp = x->wideForm();
                delete x;
	}
	endT = clock();
	printTime(startT, endT, 1000000, "clock()");


	startT = clock();
	for(int i=0; i<1000000; i++) {
		char * x = XyLatinStr::CreateFromUTF8("toto fdfsdfgsdg gfgdfgfd gfgfd gfd fgdfg fgfd fdg dfgfd fdg dfg fg dfg dfg fg dfg fdg fg fdg dfg fdg dg ffgfgdfgbbbvbfddddddd er er ezzzzzzzzzzerzerzerer fdgfgdfg fdgfdgfg cbbvbbvbgdrfgdfgdf fdgfg");
                delete x;
	}
	endT = clock();
	printTime(startT, endT, 1000000, "clock()");

        tmp = NULL;

	printf("\n\n=== OK ===\n\n");

}
	
