#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOMException.hpp"

#include "DeltaException.hpp"
#include "xydiff/XID_DOMDocument.hpp"
#include "xydiff/XID_map.hpp"
#include "Tools.hpp"
#include "xydiff/XyLatinStr.hpp"
#include "xyleme_DOMPrint.hpp"
#include <stdio.h>
#include <fstream>
#include <string>

#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/util/XMLString.hpp"

#include <stdarg.h>
#include <locale.h>

XERCES_CPP_NAMESPACE_USE
using namespace std;

//std::ostream& operator<< (std::ostream& target, DOMNode &toWrite);

void printInfos(DOMNode *node) {
	if (node==NULL) {
	    printf("node==NULL\n");
	    return;
	}
	
	switch(node->getNodeType()) {
		
		/* -- Element Node -- */
		
		case DOMNode::ELEMENT_NODE:
		{
			XyLatinStr name(node->getNodeName());
			printf("ELEMENT_NODE: <%s>\n", name.localForm());
			if (node->hasChildNodes()) {
				DOMNode* child=node->getFirstChild();
				while(child!=NULL) {
					printInfos(child);
					child=child->getNextSibling();
				}
			}
			printf("/ELEMENT_NODE: </%s>\n", name.localForm());
			return;
		}
		/* -- Text Node -- */
		
		case DOMNode::TEXT_NODE:
		{	
			XyLatinStr v(node->getNodeValue());
			printf("TEXT_NODE: [XyLatinStr.localForm()] =%s\n", v.localForm());
			fflush(stdout);
			const char *c=v.localForm();
			while (*c != '\0') {
			  int x = *(unsigned char*)c;
			  std::cout << ", "<<x;
			  c++;
			}
			std::cout << std::endl;

			XyLatinStr vl(node->getNodeValue());
			printf("TEXT_NODE: [XyLatinStr.localForm()] =%s\n", vl.localForm());
			fflush(stdout);
			const char *cl=vl.localForm();
			while (*cl != '\0') {
			  int x = *(unsigned char*)cl;
			  std::cout << ", "<<x;
			  cl++;
			}
			std::cout << std::endl;
			
			std::cout <<"           [<<node.Value()   ] ="<<node->getNodeValue()<<std::endl;

			std::cout <<"           [outputContent    ] =";
			outputContent(std::cout, node->getNodeValue());
			std::cout << std::endl;
			
			std::cout <<"           [(char*)          ] ="<<(char*)node->getNodeValue()<<endl;
			
			const XMLCh *p = node->getNodeValue();
			while (*p!='\0')
			  {
			  int x = *((int*)p);
			  std::cout <<", "<<x;
			  p++;
			  }
			std::cout<<std::endl;
			fflush(stdout);
			return;
		}
		case DOMNode::CDATA_SECTION_NODE:
		{	
			XyLatinStr v(node->getNodeValue());
			printf("CDATA_SECTION_NODE: %s\n", v.localForm());
			return;
		}
		case DOMNode::COMMENT_NODE:
		{	
			XyLatinStr v(node->getNodeValue());
			printf("COMMENT_NODE: %s\n", v.localForm());
			return;
		}
		/* -- Other Types -- */

		case DOMNode::ENTITY_REFERENCE_NODE:
			THROW_AWAY(("Unsupported node type Entity - can't compare"));
			break;

		default:
			THROW_AWAY(("Unknown node type %d - can't compare", (int)node->getNodeType() ));
			break;
		
		}
	}

int main(int argc, char **argv) {
  
	if (argc<2) {
	  cerr << "usage: exec file.xml" << endl ;
		return(0);
		}
	if (setlocale(LC_ALL, "fr_FR")) {
		printf("locale set to fr_FR\n");
	}
  try {
    XMLPlatformUtils::Initialize();
    }
  catch(const XMLException& toCatch) {
    cerr << "Error during Xerces-c Initialization.\n"
	       << "  Exception message:" << XyLatinStr(toCatch.getMessage()).localForm() << endl;
    }
	
	try {
		printf("Opening file <%s>\n", argv[1]);
		XID_DOMDocument* d1 = new XID_DOMDocument(argv[1], false);
    DOMNode* root = d1->getDocumentElement();
    printInfos(root);
    }
	catch(const VersionManagerException &e ) {
	  cerr << e << endl ;
		}
	catch(const DOMException &e ) {
	  cerr << "DOMException, code=" << e.code << endl ;
		cerr << "DOMException, message=" << e.msg << endl ;
		}	
	cout << "Terminated." << endl ;
	}
	
