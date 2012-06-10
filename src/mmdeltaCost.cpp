#include "xydiff/XyLatinStr.hpp"

#include "DeltaException.hpp"
#include "xydiff/XID_map.hpp"
#include "xydiff/XID_DOMDocument.hpp"
#include "Tools.hpp"

#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOMException.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/dom/DOMElement.hpp"

#include <string>
#include <stdio.h>
#include <fstream>

#include <vector>

#define DELTAXMLATTR "deltaxml:delta"
#define DELTAXMLUPDTXT "deltaxml:PCDATAmodify"

XERCES_CPP_NAMESPACE_USE 

XID_DOMDocument* mmdeltaDoc ;

int getMMDeltaCost(DOMNode* operation) {
	if (operation==NULL) return 0;
	int myCost = 0;
	
	while (operation!=NULL) {
		if (operation->getNodeType()==DOMNode::ELEMENT_NODE) {
			if (XMLString::equals(operation->getNodeName(),XMLString::transcode("update"))) {
				printf("update (+1)\n");
				myCost++;
				}
			else if ((XMLString::equals(operation->getNodeName(),XMLString::transcode("delete")))||(XMLString::equals(operation->getNodeName(),XMLString::transcode("insert")))) {
				DOMNode* treeNode = operation->getFirstChild();
				bool found=false;
				while(!found) {
					if (treeNode==NULL) THROW_AWAY(("bad formed delta: tree node not found"));
					if ((treeNode->getNodeType()==DOMNode::ELEMENT_NODE)&&(XMLString::equals(treeNode->getNodeName(),XMLString::transcode("tree")))) found=true;
					else treeNode=treeNode->getNextSibling();
					}
				int count = mmdeltaDoc->getSubtreeNodeCount(treeNode)-1;
				if (XMLString::equals(operation->getNodeName(), XMLString::transcode("delete"))) printf("delete %d nodes\n", count);
				if (XMLString::equals(operation->getNodeName(), XMLString::transcode("insert"))) printf("insert %d nodes\n", count);
				myCost += count ;
				}
			else THROW_AWAY(("unknown operation node"));
			}
		
		operation = operation->getNextSibling();
		}
		
	return myCost;
	}

int main(int argc, char **argv) {
 
  if (argc<2) {
	  std::cerr << "usage: exec mmdelta_filename" << std::endl ;
	  return(0);
		}

  try {
    XMLPlatformUtils::Initialize();
    }

  catch(const XMLException& toCatch) {
    std::cerr << "Error during Xerces-c Initialization.\n"
	       << "  Exception message:" << XyLatinStr(toCatch.getMessage()).localForm() << std::endl;
    }

  std::string mmdeltaFile = argv[1];
	
	try {
		mmdeltaDoc = new XID_DOMDocument(mmdeltaFile.c_str(), false);
		
	  DOMElement* deltaRoot = mmdeltaDoc->getDocumentElement() ;
	  if (deltaRoot==NULL) throw VersionManagerException("Data Error", "testDeltaReverse", "deltaRoot is NULL");
		
		if (!XMLString::equals(deltaRoot->getNodeName(), XMLString::transcode("delta"))) THROW_AWAY(("root is not <delta>"));
		
		int cost = 0;
		if (deltaRoot->hasChildNodes()) {
			DOMNode* firstOp = deltaRoot->getFirstChild();
			cost = getMMDeltaCost(firstOp);
			}
		printf("EDITING COST %d\n", cost);
		
	  std::cout << "Terminated." << std::endl ;
	  }
	catch( const VersionManagerException &e ) {
	  std::cerr << e << std::endl ;
		}
	catch( const DOMException &e ) {
	  std::cerr << "DOMException, code=" << e.code << std::endl ;
		std::cerr << "DOMException, message=" << e.msg << std::endl ;
		}	
	return(0);
	}
