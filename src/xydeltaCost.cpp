// toto
#include "XyDiff/include/XyLatinStr.hpp"

#include "XyDiff/DeltaException.hpp"
#include "XyDiff/include/XID_map.hpp"
#include "XyDiff/include/XID_DOMDocument.hpp"
#include "XyDiff/Tools.hpp"

#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOMException.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/util/XMLString.hpp"

#include <string>
#include <stdio.h>
#include <fstream>

#include <vector>

#define DELTAXMLATTR "deltaxml:delta"
#define DELTAXMLUPDTXT "deltaxml:PCDATAmodify"

XID_DOMDocument* xydeltaDoc ;
XID_DOMDocument* sourceDoc ;

struct MoveInfo {
	XID_t rootXid;
	int subtreeSize;
	};
	
std::vector<MoveInfo> moveInfoList ;

bool nomove = false ;

void createMoveInfoList(xercesc_2_2::DOMNode* operation) {
	while (operation!=NULL) {
	
		if (operation->getNodeName() == L"d") {
			xercesc_2_2::DOMNode* moveAttr = operation->getAttributes()->getNamedItem(L"move");
			if (moveAttr!=NULL) {
				struct MoveInfo myMoveInfo ;
				myMoveInfo.rootXid = XID_map::getXidFromAttribute(operation, L"xm", true);
				xercesc_2_2::DOMNode* dataNode = sourceDoc->getXidMap().getNodeWithXID(myMoveInfo.rootXid);
				myMoveInfo.subtreeSize = sourceDoc->getSubtreeNodeCount(dataNode);
				printf("move found: rootXid=%d, subtreeSize=%d\n", (int)myMoveInfo.rootXid, myMoveInfo.subtreeSize);
				moveInfoList.push_back(myMoveInfo);
				}
			}
		operation = operation->getNextSibling();
		}
	}

bool findMoveInfo(struct MoveInfo *myMoveInfo) {
	xercesc_2_2::DOMNode* node = sourceDoc->getXidMap().getNodeWithXID(myMoveInfo->rootXid);
	while ((node!=NULL)&&(node!=sourceDoc->getDocumentElement())) {
		XID_t theXid = sourceDoc->getXidMap().getXIDbyNode(node);
		for(unsigned int i=0; i<moveInfoList.size(); i++) {
			if (moveInfoList[i].rootXid==theXid) {
				printf("found move from rooted at %d which is ancestor of %d\n", (int)theXid, (int)myMoveInfo->rootXid);
				myMoveInfo->subtreeSize = moveInfoList[i].subtreeSize;
				return true;
				}
			}
		node = node->getParentNode();
		}
	return false;
	}

int getXyDeltaCost(xercesc_2_2::DOMNode* operation) {
	if (operation==NULL) return 0;
	int myCost = 0;
	
	if (xercesc_2_2::XMLString::equals(operation->getNodeName(),L"d")) {
		xercesc_2_2::DOMNode* updateAttr = operation->getAttributes()->getNamedItem(L"update");
		xercesc_2_2::DOMNode* moveAttr = operation->getAttributes()->getNamedItem(L"move");

		// Delete
		if ((updateAttr==NULL)&&(moveAttr==NULL)) {
			int dataSize = 0;
			if (operation->hasChildNodes()) {
				xercesc_2_2::DOMNode* data = operation->getFirstChild();
				dataSize=xydeltaDoc->getSubtreeNodeCount(data);
				}
			printf("delete %d nodes\n", dataSize);
			myCost += dataSize;
			if (nomove) {
				struct MoveInfo myMoveInfo;
				myMoveInfo.rootXid = XID_map::getXidFromAttribute(operation, L"par", false);
				if (findMoveInfo(&myMoveInfo)) {
					printf("  substract cost of actual operation from ancestor 'move' operation: myCost-=2x%d\n", dataSize);
					myCost -= 2*dataSize ;
					}
				}
			}

		// Move (from)
		else if (updateAttr==NULL) {
			if (nomove) {
				struct MoveInfo myMoveInfo ;
				myMoveInfo.rootXid = XID_map::getXidFromAttribute(operation, L"xm", true);
				if (!findMoveInfo(&myMoveInfo)) THROW_AWAY(("internal inconsistency is move info list"));
				printf("move-from transformed into delete %d nodes\n", myMoveInfo.subtreeSize);
				myCost += myMoveInfo.subtreeSize;
				struct MoveInfo ancMoveInfo ;
				ancMoveInfo.rootXid = XID_map::getXidFromAttribute(operation, L"par", false);
				if (findMoveInfo(&ancMoveInfo)) {
					printf("  substract cost of actual operation from ancestor 'move' operation: myCost-=2x%d\n", myMoveInfo.subtreeSize);
					myCost -= 2*myMoveInfo.subtreeSize ;
					}
				}
			else {
				printf("skipping move from\n");
				}
			}

		// Update
		else if (moveAttr==NULL) {
			printf("skipping update from\n");
			}
		}

	if (xercesc_2_2::XMLString::equals(operation->getNodeName(),L"i")) {
		xercesc_2_2::DOMNode* updateAttr = operation->getAttributes()->getNamedItem(L"update");
		xercesc_2_2::DOMNode* moveAttr = operation->getAttributes()->getNamedItem(L"move");
		
		// Insert
		if ((updateAttr==NULL)&&(moveAttr==NULL)) {
			int dataSize = 0;
			if (operation->hasChildNodes()) {
				xercesc_2_2::DOMNode* data = operation->getFirstChild();
				dataSize=xydeltaDoc->getSubtreeNodeCount(data);
				}
			printf("delete %d nodes\n", dataSize);
			myCost += dataSize;
			}
			
		// Move (to)
		else if (updateAttr==NULL) {
			if (nomove) {
				struct MoveInfo myMoveInfo ;
				myMoveInfo.rootXid = XID_map::getXidFromAttribute(operation, L"xm", true);
				if (!findMoveInfo(&myMoveInfo)) THROW_AWAY(("inconsistent move info list"));
				printf("move to transformed into insert %d nodes\n", myMoveInfo.subtreeSize);
				myCost += myMoveInfo.subtreeSize;
				}
			else {
				printf("move to (+1)\n");
				myCost++;
				}
			}
			
		// Update
		else if (moveAttr==NULL) {
			printf("update to (+1)\n");
			myCost++;
			}
		}
	
	myCost += getXyDeltaCost(operation->getNextSibling());
	return myCost;
	}

int main(int argc, char **argv) {
 
	if (argc<2) {
		std::cerr << "usage: exec xydelta_filename [nomove]" << std::endl ;
		return(0);
	}

	try {
		xercesc_2_2::XMLPlatformUtils::Initialize();
	}

	catch(const xercesc_2_2::XMLException& toCatch) {
		std::cerr << "Error during Xerces-c Initialization.\n"
		     << "  Exception message:" << XyLatinStr(toCatch.getMessage()).localForm() << std::endl;
	}

	std::string xydeltaFile = argv[1];
	if ((argc>=3)&&(strcmp(argv[2], "nomove")==0)) {
		printf("nomove mode enabled - move cost is now insert+delete cost\n");
		nomove = true;
		}
	
	try {
		xydeltaDoc = new XID_DOMDocument(xydeltaFile.c_str(), false);
		
		xercesc_2_2::DOMElement* deltaRoot = xydeltaDoc->getDocumentElement() ;
		if (deltaRoot==NULL) throw VersionManagerException("Data Error", "testDeltaReverse", "deltaRoot is NULL");
		
		xercesc_2_2::DOMNode* deltaElement = deltaRoot->getFirstChild();
		if (deltaElement==NULL) throw VersionManagerException("Data Error", "testDeltaReverse", "deltaElement is NULL");
		
		if (nomove) {
			xercesc_2_2::DOMNode *n = deltaElement->getAttributes()->getNamedItem(L"from");
			XyLatinStr sourceFile(n->getNodeValue());
			printf("opening source file <%s>\n", (const char*)sourceFile);
			sourceDoc = new XID_DOMDocument(sourceFile, true);
			}
		
		int cost = 0;
		if (deltaElement->hasChildNodes()) {
			xercesc_2_2::DOMNode* firstOp = deltaElement->getFirstChild();
			if (nomove) createMoveInfoList(firstOp);
			cost = getXyDeltaCost(firstOp);
			}
		printf("EDITING COST %d\n", cost);
		
		std::cout << "Terminated." << std::endl ;
	}
	catch( const VersionManagerException &e ) {
		std::cerr << e << std::endl ;
	}
	catch( const xercesc_2_2::DOMException &e ) {
		std::cerr << "DOM_DOMException, code=" << e.code << std::endl ;
		std::cerr << "DOM_DOMException, message=" << e.msg << std::endl ;
	}	
	return(0);
}
