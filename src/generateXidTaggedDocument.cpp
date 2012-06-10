#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOMException.hpp"

#include "xydiff/XyLatinStr.hpp"
#include "xydiff/XID_DOMDocument.hpp"
#include "xydiff/XID_map.hpp"
#include "Tools.hpp"
#include <stdio.h>
#include <fstream>

#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"

XERCES_CPP_NAMESPACE_USE

int main(int argc, char **argv) {
  
	if (argc<2) {
	  std::cerr << "usage: exec filename" << std::endl ;
		std::cerr << "example: exec example1.xml" << std::endl ;
		std::cerr << "result: example1.xml.debugXID" << std::endl;
		return(0);
		}

  try {
    XMLPlatformUtils::Initialize();
    }
  catch(const XMLException& toCatch) {
    std::cerr << "Error during Xerces-c Initialization.\n"
	       << "  Exception message:" << XyLatinStr(toCatch.getMessage()).localForm() << std::endl;
    }
	
	try {
		printf("Opening file <%s>\n", argv[1]);
		XID_DOMDocument* d = new XID_DOMDocument(argv[1]);
	
		DOMElement* root = d->getDocumentElement();
		if (root!=NULL) Restricted::XidTagSubtree(d, root);
		
		std::string fileWithXID = argv[1] ;
	  fileWithXID += "_XidTagged.xml" ;

		printf("Saving with XID as file <%s>\n", fileWithXID.c_str());
	  d->SaveAs(fileWithXID.c_str(), false);
	  }
	catch( const VersionManagerException &e ) {
	  std::cerr << e << std::endl ;
		}
	catch( const DOMException &e ) {
	  std::cerr << "DOM_DOMException, code=" << e.code << std::endl ;
		std::cerr << "DOM_DOMException, message=" << XMLString::transcode(e.msg) << std::endl ;
		}	
	std::cout << "Terminated." << std::endl ;
	}
	


