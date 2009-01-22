#include "XyDiff/Tools.hpp"
#include "XyDiff/include/XyLatinStr.hpp"

#include <iostream>
#include "xercesc/util/XMLUni.hpp"

#include "xercesc/util/XMLString.hpp"

#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"

//--------------------------------------------------------------------------
//
//get the child position of a node in the source document
//
//--------------------------------------------------------------------------

int getPosition(xercesc_2_2::DOMNode *parent, xercesc_2_2::DOMNode *child) {
  int pos = 1;
	if (!parent->hasChildNodes()) throw VersionManagerException("getPosition()", "Parent has no child") ;
  xercesc_2_2::DOMNode* test = parent->getFirstChild();
  while (test!=child) {
    test=test->getNextSibling();
    pos++;
    }
	if (test==NULL) throw VersionManagerException("getPosition()", "Child not found");
	return pos;
  }

//--------------------------------------------------------------------------
//
// tests if the DOM_Document is a delta
//
//--------------------------------------------------------------------------

bool isDelta(const xercesc_2_2::DOMDocument *doc) {
	if ((doc!=NULL)&&(doc->hasChildNodes())) {
		xercesc_2_2::DOMElement* docRoot = doc->getDocumentElement() ;
		if (docRoot!=NULL) {
        	        if (xercesc_2_2::XMLString::equals(docRoot->getNodeName(), L"unit_delta")) {
				return true ;
			}
		}
	}
	return false;
}

//--------------------------------------------------------------------------
//
//       Version Manager Exception
//
//--------------------------------------------------------------------------

VersionManagerException::VersionManagerException(const std::string &IncStatus, const std::string &IncContext, const std::string &IncMessage) {
  status = IncStatus ;
	context = IncContext ;
	message = IncMessage ;
	}

VersionManagerException::VersionManagerException(const std::string &IncContext, const std::string &IncMessage) {
  status = "Runtime Error" ;
	context = IncContext ;
	message = IncMessage ;
	}

std::ostream &operator << (std::ostream& target, const VersionManagerException &e) {
  target << "\n\n *** *** Version Manager Exception" << std::endl ;
	target << "  -Status   : " << e.status << std::endl ;
	target << "  -Context  : " << e.context << std::endl ;
	target << "  -Messages : " << e.message << std::endl ;
	return target ;
	}

/* 
 * it transforms a DOMString into an int; if it's error returns 0 
 * problem: how to know if 0 means error code or a valid value ???
 */

int watoi(const XMLCh* str) {
	int intValue = atoi(XyLatinStr(str).localForm());
	// cout << "Tools.cxx -> watoi() : intValue=" << intValue << endl;
	return intValue;
}

/*
 * tells if a certain file exists or not 
 */

bool existsFile(const char *fileName) {
	FILE *testFile = fopen(fileName, "r");
	if ( testFile != NULL ) {
		fclose(testFile);
	}
	return (testFile != NULL);
}

