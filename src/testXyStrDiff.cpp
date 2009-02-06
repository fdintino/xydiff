#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"

#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"

#include "include/XyStrDiff.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

// For output
#include "xercesc/dom/DOMLSOutput.hpp"
#include "xercesc/dom/DOMLSSerializer.hpp"
#include "xercesc/framework/MemBufFormatTarget.hpp"

XERCES_CPP_NAMESPACE_USE

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("usage: testXyStrDiff <str1> <str2>\n");
		return(-1);
	}	

	XMLPlatformUtils::Initialize();
	
	char **strings = &argv[1];
	char *str1 = strings[0];
	char *str2 = strings[1];

	static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
	DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
	DOMDocument *doc = impl->createDocument(0, XMLString::transcode("root"), 0);
	
	DOMElement *root = doc->getDocumentElement();
	XyStrDiff *strdiff = new XyStrDiff(doc, root, str1, str2);
	
	strdiff->LevenshteinDistance();

	
	DOMLSSerializer* theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
	DOMLSOutput *theOutput = ((DOMImplementationLS*)impl)->createLSOutput();
	
	
	XMLFormatTarget *myFormatTarget = new MemBufFormatTarget();
	theOutput->setByteStream(myFormatTarget);
	
	
	try {
		if (theSerializer->getDomConfig()->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
			theSerializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
		if (theSerializer->getDomConfig()->canSetParameter(XMLUni::fgDOMXMLDeclaration, true)) 
			theSerializer->getDomConfig()->setParameter(XMLUni::fgDOMXMLDeclaration, true);		
		theSerializer->write((DOMDocument*)doc, theOutput);
	}
	catch (const XMLException& toCatch) {
		char* message = XMLString::transcode(toCatch.getMessage());
		std::cout << "Exception message is: \n" << message << std::endl;
		XMLString::release(&message);
	}
	catch (const DOMException& toCatch) {
		char* message = XMLString::transcode(toCatch.msg);
		std::cout << "Exception message is: \n" << message << std::endl;
		XMLString::release(&message);
	}
	catch (...) {
		std::cout << "Unexpected Exception" << std::endl;
	}
	
	char* theXMLString_Encoded = (char*) ((MemBufFormatTarget*)myFormatTarget)->getRawBuffer();
	std::cout << theXMLString_Encoded << std::endl;
	

	doc->release();
	return 0;
}
