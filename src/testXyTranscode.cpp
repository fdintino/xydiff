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

using namespace std;

void displayByteSequence(const XMLCh *p) {
	std::cout << "        wchar sequence = " ;
	while (*p!='\0') {
		int x = *((int*)p);
		std::cout <<", "<<x;
		p++;
	}
	std::cout << endl;
}

void displayByteSequence(const char *c) {
	std::cout << "        char sequence = " ;
	while (*c != '\0') {
		int x = *(unsigned char*)c;
		std::cout << ", "<<x;
		c++;
	}
	std::cout << endl;
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

	{
		std::cout << "1) TEST NO accentuation" << endl;
		char *s = "Gregory";
		XMLCh *g1 = xercesc_2_2::XMLString::transcode(s);
		
		std::string<XMLCh> g1str = g1 ;
		std::cout << "Unicode sequence for 'Gregory': " << endl;
		displayByteSequence(g1str.data());
		
		XyLatinStr vv(g1);
		printf("TEXT_NODE: [XyLatinStr.localForm()] =%s\n", vv.localForm());
		displayByteSequence(vv.localForm());

		XyUTF8Str vu(g1);
		printf("TEXT_NODE: [XyLatinStr.utf8()] =%s\n", vu.localForm());
		const char *cc=vu.localForm();
		displayByteSequence(cc);

		std::cout << "END TEST" << std::endl;
	}

	{
		std::cout << "2) TEST Accentuation" << endl;
		char *s = "Gregory";
		char *g = new char[100];
		strcpy(g,s);
		g[2]=233;
		XMLCh *g1 = xercesc_2_2::XMLString::transcode(g);
		
		std::string<XMLCh> g1str = g1 ;
		std::cout << "Unicode sequence for 'Grégory': " << endl;
		displayByteSequence(g1str.data());
		
		XyLatinStr vv(g1);
		printf("TEXT_NODE: [XyLatinStr.localForm()] =%s\n", vv.localForm());
		displayByteSequence(vv.localForm());

		XyUTF8Str vu(g1);
		printf("TEXT_NODE: [XyLatinStr.utf8()] =%s\n", vu.localForm());
		const char *cc=vu.localForm();
		displayByteSequence(cc);

		std::cout << "END TEST" << std::endl;
	}

	{
		std::cout << "TEST--2--" << endl;
		char *s = "Gregory";
		char *g = new char[100];
		strcpy(g,s);
		g[2]=233;

		XMLCh *g1 = new XMLCh[500];
		xercesc_2_2::XMLString::transcode(g, g1, 499);
		std::cout << "Unicode" << endl;
		const XMLCh *p = g1;
		while (*p!='\0') {
			int x = *((int*)p);
			std::cout <<", "<<x;
			p++;
		}
		
		XyLatinStr vv(g1);
		printf("TEXT_NODE: [XyLatinStr.localForm()] =%s\n", vv.localForm());
		fflush(stdout);
		const char *c=vv.localForm();
		while (*c != '\0') {
			int x = *(unsigned char*)c;
			std::cout << ", "<<x;
			c++;
		}
		std::cout << "END TEST" << std::endl;
	}


	XMLCh *c1 = L"test Grégory, et è et ê pour les accents...";
	XyUTF8Str c2(c1);
	printf("LAST_TEST = %s\n", c2.localForm());
	XyUTF8Str c3(c2.wideForm());
	printf("LAST_TEST = %s\n", c3.localForm());
	XyUTF8Str c4(c3.localForm());
	printf("LAST_TEST = %s\n", c4.localForm());
	XyUTF8Str c5(c4.localForm());
	printf("LAST_TEST = %s\n", c5.localForm());
	XyUTF8Str c6(c5.wideForm());
	printf("LAST_TEST = %s\n", c6.localForm());
	XyUTF8Str c7(c6.localForm());
	printf("LAST_TEST = %s\n", c7.localForm());
	
	{
		printf("\nTEST XyUTF8Str (part 1)\n");
		std::string<XMLCh> s1 = L"teste";
		while(s1.size()<100*1000) {
			s1 += L"Grégory Fête Très ";
		}
		printf("Len XMLCh=%u\n", xercesc_2_2::XMLString::stringLen(s1.data()));
		XyLatinStr cc1(s1.data());
		printf("Len XMLCh=%u\n", cc1.wideFormSize());
		XyUTF8Str cc2(cc1.wideForm());
		printf("Len XMLCh=%u\n", cc2.wideFormSize());
		XyUTF8Str cc3(cc2.localForm());
		printf("Len XMLCh=%u\n", cc3.wideFormSize());
		XyUTF8Str cc4(cc3.localForm());
		printf("Len XMLCh=%u\n", cc4.wideFormSize());
		XyUTF8Str cc5(cc4.wideForm());
		printf("Len XMLCh=%u\n", cc5.wideFormSize());
		XyUTF8Str cc6(cc5.localForm());
		printf("Len XMLCh=%u\n", cc6.wideFormSize());
		XyUTF8Str cc7(cc6.wideForm());
		printf("Len UTF8 =%u\n", strlen(cc7.localForm()));
		printf("Len XMLCh=%u\n", xercesc_2_2::XMLString::stringLen(cc7.wideForm()));
		fflush(stdout);
	}

	{
		printf("\nTEST XyUTF8Str (part 2)\n");
		std::string s1 = "testé";
		while(s1.size()<100*1000) {
			s1 += "Grégory Fête Très ";
		}
		printf("Len Latin=%u\n", strlen(s1.c_str()));
		XyLatinStr cc1(s1.data());
		XyUTF8Str cc2(cc1.wideForm());
		XyUTF8Str cc3(cc2.localForm());
		XyUTF8Str cc4(cc3.localForm());
		XyUTF8Str cc5(cc4.wideForm());
		XyUTF8Str cc6(cc5.localForm());
		XyUTF8Str cc7(cc6.wideForm());
		printf("Len XMLCh=%u\n", xercesc_2_2::XMLString::stringLen(cc7.wideForm()));
		printf("Len UTF8 =%u\n", strlen(cc7.localForm()));
		fflush(stdout);
	}

	{
		printf("\nTEST XyLatinStr\n");
		std::string s1 = "testé";
		while(s1.size()<100*1000) {
			s1 += "Grégory Fête Très ";
		}
		printf("Len Latin=%u\n", strlen(s1.c_str()));
		XyLatinStr cc1(s1.data());
		printf("Len Latin=%u\n", cc1.localFormSize());
		XyLatinStr cc2(cc1.localForm());
		printf("Len Latin=%u\n", cc2.localFormSize());
		XyLatinStr cc3(cc2.localForm());
		printf("Len Latin=%u\n", cc3.localFormSize());
		XyUTF8Str pre_cc4(cc3.wideForm());
		printf("(pre)Len Latin=%u\n", pre_cc4.localFormSize());
		XyLatinStr cc4(cc3.wideForm());
		printf("Len Latin=%u\n", cc4.localFormSize());
		XyLatinStr cc5(cc4.wideForm());
		printf("Len Latin=%u\n", cc5.localFormSize());
		XyLatinStr cc6(cc5.localForm());
		printf("Len Latin=%u\n", cc6.localFormSize());
		XyLatinStr cc7(cc6.wideForm());
		printf("Len XMLCh=%u\n", xercesc_2_2::XMLString::stringLen(cc7.wideForm()));
		printf("Len Latin=%u\n", strlen(cc7.localForm()));
		fflush(stdout);
	}
	
	{
		printf("\nTest strings containing NULL characters\n");
		std::string s1("grégory\0fête\0très\0fin.", 22);
		printf("Len std::string=%u (%u)\n", s1.size(), s1.length());
		XyLatinStr cc1(s1.data(), s1.size());
		printf("Len XMLCh=%u\n", cc1.wideFormSize());
		printf("Len Latin=%u\n", cc1.localFormSize());

		XyUTF8Str cc2(cc1.wideForm(), cc1.wideFormSize());
		XyUTF8Str cc3(cc2.localForm(), cc2.localFormSize());
		XyUTF8Str cc4(cc3.wideForm(), cc3.wideFormSize(), XyStr::NO_FAST_OPTION);
		XyUTF8Str cc5(cc4.wideForm(), cc4.wideFormSize(), XyStr::NO_SOURCE_COPY+XyStr::NO_BUFFER_ADJUST);
		XyUTF8Str cc6(cc5.localForm(), cc5.localFormSize());
		XyUTF8Str cc7(cc6.wideForm(), cc6.wideFormSize());
		printf("Len XMLCh=%u\n", cc7.wideFormSize());
		printf("Len UTF8 =%u, s=%s\n", cc7.localFormSize(), cc7.localForm());
		XyLatinStr cc8(cc7.wideForm(), cc7.wideFormSize());
		printf("Len Latin =%u s=%s\n", cc8.localFormSize(), cc8.localForm());
		fflush(stdout);
	}


	{
		printf("\nTest Transcoding DOCUMENTS\n\n");
		std::string<XMLCh> s1(L"<?xml version=\"1.0\" encoding=\"UTF-32\" ?><root><a/><b/></root>");
		s1.push_back(xercesc_2_2::chNull);
		printf("Len S1.size=%u, S1=%s\n", s1.size(), XyLatinStr(s1.data()).localForm());
		
		XyUTF8Document su(s1.data());
		printf("Len SU.size=%u, SU=%s\n", su.localFormSize(), su.localForm());
	}
	printf("\n\n=== OK ===\n\n");

}
	
