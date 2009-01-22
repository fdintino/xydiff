#include "XyDiff/include/XyLatinStr.hpp"

#include "XyDiff/DeltaException.hpp"
#include "XyDiff/include/XID_map.hpp"
#include "XyDiff/include/XID_DOMDocument.hpp"
#include "XyDiff/Tools.hpp"

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

XID_DOMDocument* mmdeltaDoc ;

int getMMDeltaCost(xercesc_2_2::DOMNode* operation) {
	if (operation==NULL) return 0;
	int myCost = 0;
	
	while (operation!=NULL) {
		if (operation->getNodeType()==xercesc_2_2::DOMNode::ELEMENT_NODE) {
			if (xercesc_2_2::XMLString::equals(operation->getNodeName(),L"update")) {
				printf("update (+1)\n");
				myCost++;
				}
			else if ((xercesc_2_2::XMLString::equals(operation->getNodeName(),L"delete"))||(xercesc_2_2::XMLString::equals(operation->getNodeName(),L"insert"))) {
				xercesc_2_2::DOMNode* treeNode = operation->getFirstChild();
				bool found=false;
				while(!found) {
					if (treeNode==NULL) THROW_AWAY(("bad formed delta: tree node not found"));
					if ((treeNode->getNodeType()==xercesc_2_2::DOMNode::ELEMENT_NODE)&&(xercesc_2_2::XMLString::equals(treeNode->getNodeName(),L"tree"))) found=true;
					else treeNode=treeNode->getNextSibling();
					}
				int count = mmdeltaDoc->getSubtreeNodeCount(treeNode)-1;
				if (xercesc_2_2::XMLString::equals(operation->getNodeName(), L"delete")) printf("delete %d nodes\n", count);
				if (xercesc_2_2::XMLString::equals(operation->getNodeName(), L"insert")) printf("insert %d nodes\n", count);
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
    xercesc_2_2::XMLPlatformUtils::Initialize();
    }

  catch(const xercesc_2_2::XMLException& toCatch) {
    std::cerr << "Error during Xerces-c Initialization.\n"
	       << "  Exception message:" << XyLatinStr(toCatch.getMessage()).localForm() << std::endl;
    }

  std::string mmdeltaFile = argv[1];
	
	try {
		mmdeltaDoc = new XID_DOMDocument(mmdeltaFile.c_str(), false);
		
	  xercesc_2_2::DOMElement* deltaRoot = mmdeltaDoc->getDocumentElement() ;
	  if (deltaRoot==NULL) throw VersionManagerException("Data Error", "testDeltaReverse", "deltaRoot is NULL");
		
		if (!xercesc_2_2::XMLString::equals(deltaRoot->getNodeName(), L"delta")) THROW_AWAY(("root is not <delta>"));
		
		int cost = 0;
		if (deltaRoot->hasChildNodes()) {
			xercesc_2_2::DOMNode* firstOp = deltaRoot->getFirstChild();
			cost = getMMDeltaCost(firstOp);
			}
		printf("EDITING COST %d\n", cost);
		
	  std::cout << "Terminated." << std::endl ;
	  }
	catch( const VersionManagerException &e ) {
	  std::cerr << e << std::endl ;
		}
	catch( const xercesc_2_2::DOMException &e ) {
	  std::cerr << "DOMException, code=" << e.code << std::endl ;
		std::cerr << "DOMException, message=" << e.msg << std::endl ;
		}	
	return(0);
	}
