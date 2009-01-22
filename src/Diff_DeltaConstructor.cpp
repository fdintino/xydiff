#include "include/XyLatinStr.hpp"
#include "include/XyUTF8Str.hpp"
#include "Diff_DeltaConstructor.hpp"

#include "CommonSubSequenceAlgorithms.hpp"
#include "Tools.hpp"
#include "include/XID_map.hpp"
#include "Diff_NodesManager.hpp"

#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMAttr.hpp"
#include "xercesc/util/XMLString.hpp"

//#define SKIP_UNIMPORTANT_MOVE 1

#include <stdio.h>
#include <map>
#include <queue>
#include <list>
#include <math.h>
#include <fstream>
#include "infra/general/hash_map.hpp"

unsigned long long int   clocksCDDinitDomDelta      ;
unsigned long long int   clocksCDDprepareVectors    ;
unsigned long long int   clocksCDDsaveXidMap        ;
unsigned long long int   clocksCDDcomputeWeakMove   ;
unsigned long long int   clocksCDDdetectUpdate      ;
unsigned long long int   clocksCDDaddAttributeOps   ;
unsigned long long int   clocksConstructDOMDelta    ;

bool _XyDiff_DontSaveXidmapToFile = false;

XID_DOMDocument *DeltaConstructor::getDeltaDocument(void) {
	return deltaDoc ;
	}

DeltaConstructor::DeltaConstructor(
		class NodesManager *incMappings,
		const char *from,
		XID_DOMDocument* fromXDD,
		const char *to,
		XID_DOMDocument* toXDD,
		bool IncIgnoreUnimportantData) {
		
	unsigned long long int localStart = rdtsc() ;

	v0XML        = fromXDD ;
	v1XML        = toXDD ;
	v0filename   = from ;
	v1filename   = to ;
	nodesManager = incMappings ;
	
	moveCount    = 0 ;
	updateCount  = 0 ;
	
	ignoreUnimportantData = IncIgnoreUnimportantData ;
	
	deltaDoc     = XID_DOMDocument::createDocument() ;
	XMLCh tempStr[100];
	XMLCh tempStr2[100];

	xercesc_3_0::XMLString::transcode("unit_delta", tempStr, 99);
	xercesc_3_0::DOMElement* deltaRoot = deltaDoc->createElement(tempStr);
	deltaRoot->setAttributeNS(
		xercesc_3_0::XMLString::transcode("http://www.w3.org/2000/xmlns/"),
		xercesc_3_0::XMLString::transcode("xmlns:xyd"),
		xercesc_3_0::XMLString::transcode("http://www.nflc.org/schema/xydiff/"));
	deltaDoc->appendChild( (xercesc_3_0::DOMNode*) deltaRoot );

	xercesc_3_0::XMLString::transcode("t", tempStr, 99);	
	xercesc_3_0::DOMElement* tElem = deltaDoc->createElement(tempStr);

	xercesc_3_0::XMLString::transcode("from", tempStr, 99);
	tElem->setAttribute(tempStr, XyLatinStr(v0filename.c_str()) );

	xercesc_3_0::XMLString::transcode("to", tempStr, 99);
	tElem->setAttribute(tempStr, XyLatinStr(v1filename.c_str()) );

	xercesc_3_0::XMLString::transcode("IgnoreBlankTexts", tempStr, 99);
	xercesc_3_0::XMLString::transcode("yes", tempStr2, 99);
	if (ignoreUnimportantData) tElem->setAttribute(tempStr,tempStr2);
	deltaRoot->appendChild( tElem );

	scriptRoot = tElem ;
	
	clocksCDDinitDomDelta += rdtsc() - localStart ;
	}

/*****************************************************
 *         ++++ Construct Delete Script ++++         *
 *****************************************************/

void DeltaConstructor::ConstructDeleteScript( int v0nodeID, bool ancestorDeleted ) {

	class AtomicInfo & myAtomicInfo = nodesManager->v0node[ v0nodeID ] ;
	xercesc_3_0::DOMNode* node = nodesManager->v0nodeByDID[ v0nodeID ] ;

	XMLCh tempStrA[100];
	XMLCh tempStrB[100];
	// Apply to Children first - note that they must be enumerated in reverse order

	std::vector<int> childList ;
	int child= myAtomicInfo.firstChild ;
	while(child) {
		childList.push_back( child );
		child = nodesManager->v0node[child].nextSibling ;
		}
	
	for(int i=childList.size()-1; i>=0; i--) {
		// ConstructDeleteScript( childList[i], ancestorDeleted||(myAtomicInfo.myEvent==AtomicInfo::DELETED));
		ConstructDeleteScript( childList[i], (myAtomicInfo.myEvent==AtomicInfo::DELETED));
		}

	// Apply to Self
	
	switch( myAtomicInfo.myEvent ) {
	
		case AtomicInfo::NOP:
			vddprintf(("nop for node %d\n", myAtomicInfo.myID));
			break ;
			
		case AtomicInfo::DELETED:
		case AtomicInfo::STRONGMOVE:
		case AtomicInfo::WEAKMOVE:
		case AtomicInfo::UPDATE_OLD:

			if ((myAtomicInfo.myEvent==AtomicInfo::DELETED)&&(ancestorDeleted)) {
				vddprintf(("skipping op 'delete node %d' because subtree is deleted starting from one of its ancestor\n", myAtomicInfo.myID));
				}

			else {
				xercesc_3_0::DOMNode* parentNode = node->getParentNode() ;
				std::string myXidMap ;
				if (myAtomicInfo.myEvent==AtomicInfo::DELETED) {
					myXidMap = nodesManager->v0doc->getXidMap().String( node );
					}
				else {
			 		char xidstr[10] ;
					sprintf(xidstr,"(%d)", (int)nodesManager->v0doc->getXidMap().getXIDbyNode( node ) );
					myXidMap = std::string( xidstr );
					}
				XID_t parentXID = nodesManager->v0doc->getXidMap().getXIDbyNode( parentNode ) ;
				int myPosition = myAtomicInfo.myPosition ; //getPosition( parentNode, node );

				char parXID_str[10] ;
				char pos_str[10] ;
				sprintf(parXID_str,"%d", (int)parentXID );
				sprintf(pos_str, "%d", myPosition);
				xercesc_3_0::XMLString::transcode("d", tempStrA, 99);
				xercesc_3_0::DOMElement* elem = deltaDoc->createElement(tempStrA);
				xercesc_3_0::XMLString::transcode("par", tempStrA, 99);
				elem->setAttribute(tempStrA, XyLatinStr( parXID_str) );
				xercesc_3_0::XMLString::transcode("pos", tempStrA, 99);
				elem->setAttribute(tempStrA, XyLatinStr(pos_str) );
				xercesc_3_0::XMLString::transcode("xm", tempStrA, 99);
				elem->setAttribute(tempStrA,  XyLatinStr( myXidMap.c_str()) );

				switch(myAtomicInfo.myEvent) {
					case AtomicInfo::DELETED: {
						xercesc_3_0::DOMNode* contentNode = deltaDoc->importNode( node, true );
						elem->appendChild ( contentNode );
						}
						break;
					case AtomicInfo::STRONGMOVE:
					case AtomicInfo::WEAKMOVE: {
						// Erase moved subtree so that any parent deleted operation won't take it
                                          //xercesc_3_0::DOMNode* movedNode = parentNode->removeChild( node );
                                          parentNode->removeChild( node );
						xercesc_3_0::XMLString::transcode("move", tempStrA, 99);
						xercesc_3_0::XMLString::transcode("yes", tempStrB, 99);
						elem->setAttribute(tempStrA, tempStrB);
						moveCount++ ;
						}
						break;
					case AtomicInfo::UPDATE_OLD: {
						xercesc_3_0::DOMNode* contentNode = deltaDoc->importNode( node, true );
						elem->appendChild ( contentNode );
						xercesc_3_0::XMLString::transcode("update", tempStrA, 99);
						xercesc_3_0::XMLString::transcode("yes", tempStrB, 99);
						elem->setAttribute(tempStrA, tempStrB);
						updateCount++ ;
						}
						break;
					default:
						throw VersionManagerException("Internal Error", "ConstructDeleteScript", "Program can't possibly be here");
					}
				
				vddprintf(("<d xm=""%s"" par=""%d"" pos=""%d"" %s%s%s/>\n",
					myXidMap.c_str(),
					(int)parentXID,
					myPosition,
					(myAtomicInfo.myEvent==AtomicInfo::STRONGMOVE)?"strongmove":"",
					(myAtomicInfo.myEvent==AtomicInfo::WEAKMOVE)?"weakmove":"",
					(myAtomicInfo.myEvent==AtomicInfo::UPDATE_OLD)?"update":""
					));

				if ((!ignoreUnimportantData)||(!myAtomicInfo.isUnimportant)) {
					scriptRoot->appendChild( elem );
					}
				}
			break ;
			
		case AtomicInfo::UPDATE_NEW:
		case AtomicInfo::INSERTED:
		default:
			fprintf(stderr, "Bad value myEvent=%d in old tree\n", myAtomicInfo.myEvent);
			break;
		}

	}

/*****************************************************
 *         ++++ Construct Insert Script ++++         *
 *****************************************************/

xercesc_3_0::DOMNode* DeltaConstructor::deltaDoc_ImportInsertTree( int v1nodeID, std::vector<XID_t> &xidList ) {
	class AtomicInfo & myAtomicInfo = nodesManager->v1node[ v1nodeID ] ;
	
	xercesc_3_0::DOMNode* node = nodesManager->v1nodeByDID[ v1nodeID ] ;
	xercesc_3_0::DOMNode* newNode = deltaDoc->importNode( node, false );
	
	// Now apply to Children
	
	int child = myAtomicInfo.firstChild ;
	while( child ) {
		class AtomicInfo & myChildInfo = nodesManager->v1node[child] ;
		
		// If Children is to be inserted
		if ((myChildInfo.myEvent==AtomicInfo::INSERTED)
		     ||(myChildInfo.myEvent==AtomicInfo::UPDATE_NEW)) {
			xercesc_3_0::DOMNode* childNode = deltaDoc_ImportInsertTree(child, xidList);
			newNode->appendChild(childNode);
			}
			
		// Next Sibling
		child = myChildInfo.nextSibling ;
		}
	
	xidList.push_back( nodesManager->v1doc->getXidMap().getXIDbyNode(node) );
	return newNode ;
	}

void DeltaConstructor::ConstructInsertScript( int v1nodeID, bool ancestorInserted ) {
	
	class AtomicInfo & myAtomicInfo = nodesManager->v1node[ v1nodeID ] ;
	xercesc_3_0::DOMNode* node = nodesManager->v1nodeByDID[ v1nodeID ] ;

	XMLCh tempStrA[100];
	XMLCh tempStrB[100];
	// Apply to Self first
	
	switch( myAtomicInfo.myEvent ) {
	
		case AtomicInfo::NOP:
			vddprintf(("nop for node %d\n", myAtomicInfo.myID));
			break ;
			
		case AtomicInfo::INSERTED:
		case AtomicInfo::STRONGMOVE:
		case AtomicInfo::WEAKMOVE:
		case AtomicInfo::UPDATE_NEW:

			if ((myAtomicInfo.myEvent==AtomicInfo::INSERTED)&&(ancestorInserted)) {
				vddprintf(("skipping op 'insert node %d' because subtree is inserted starting from one of its ancestor\n", myAtomicInfo.myID));
				}

			else {
				xercesc_3_0::DOMNode* parentNode = node->getParentNode() ;
				XID_t parentXID = nodesManager->v1doc->getXidMap().getXIDbyNode( parentNode );
				int myPosition = myAtomicInfo.myPosition ; // getPosition( parentNode, node );

				char parXID_str[10] ;
				char pos_str[10] ;
				sprintf(parXID_str,"%d", (int)parentXID );
				sprintf(pos_str, "%d", myPosition);

				xercesc_3_0::XMLString::transcode("i", tempStrA, 99);
				xercesc_3_0::DOMElement* elem = deltaDoc->createElement(tempStrA);
				xercesc_3_0::XMLString::transcode("par", tempStrA, 99);
				elem->setAttribute(tempStrA, XyLatinStr(parXID_str) );
				xercesc_3_0::XMLString::transcode("pos", tempStrA, 99);
				elem->setAttribute(tempStrA, XyLatinStr(pos_str) );

				switch(myAtomicInfo.myEvent) {
					case AtomicInfo::INSERTED: {
						// DOM_Node contentNode = deltaDoc.importNode( node, true );
						std::vector<XID_t> xidList ;
						xercesc_3_0::DOMNode* contentNode = deltaDoc_ImportInsertTree( v1nodeID, xidList );
						elem->appendChild ( contentNode );
						xercesc_3_0::XMLString::transcode("xm", tempStrA, 99);
						elem->setAttribute(tempStrA,  XyLatinStr(nodesManager->v1doc->getXidMap().StringFromList(xidList).c_str()) );
						}
						break;
					case AtomicInfo::STRONGMOVE:
					case AtomicInfo::WEAKMOVE: {
						// Erase moved subtree so that any parent inserted operation won't take it
                                          //xercesc_3_0::DOMNode* movedNode = parentNode->removeChild( node );
                                          parentNode->removeChild( node );
						xercesc_3_0::XMLString::transcode("move", tempStrA, 99);
						xercesc_3_0::XMLString::transcode("yes",  tempStrB, 99);
						elem->setAttribute(tempStrA, tempStrB);
						moveCount-- ;
			 			char xidstr[10] ;
						sprintf(xidstr,"(%d)", (int)nodesManager->v1doc->getXidMap().getXIDbyNode( node ));
						xercesc_3_0::XMLString::transcode("xm", tempStrA, 99);
						elem->setAttribute(tempStrA,  XyLatinStr(xidstr) );
						}
						break;
					case AtomicInfo::UPDATE_NEW: {
						// DOM_Node contentNode = deltaDoc.importNode( node, true );
						std::vector<XID_t> xidList ;
						xercesc_3_0::DOMNode* contentNode = deltaDoc_ImportInsertTree( v1nodeID, xidList );
						elem->appendChild ( contentNode );
						xercesc_3_0::XMLString::transcode("xm", tempStrA, 99);
						elem->setAttribute(tempStrA,  XyLatinStr(nodesManager->v1doc->getXidMap().StringFromList(xidList).c_str()) );
						
						xercesc_3_0::XMLString::transcode("update", tempStrA, 99);
						xercesc_3_0::XMLString::transcode("yes",  tempStrB, 99);
						elem->setAttribute(tempStrA, tempStrB);
						updateCount-- ;
						}
						break;
					default:
						throw VersionManagerException("Internal Error", "ConstructDeleteScript", "Program can't possibly be here");
					}

#if 0
				vddprintf(("<i xm=""%s"" par=""%d"" pos=""%d"" %s%s%s/>\n",
					myXidMap.c_str(),
					(int)parentXID,
					myPosition,
					(myAtomicInfo.myEvent==AtomicInfo::STRONGMOVE)?"strongmove":"",
					(myAtomicInfo.myEvent==AtomicInfo::WEAKMOVE)?"weakmove":"",
					(myAtomicInfo.myEvent==AtomicInfo::UPDATE_NEW)?"update":""
					));
#endif

				if ((!ignoreUnimportantData)||(!myAtomicInfo.isUnimportant)) {
					scriptRoot->appendChild( elem );
					}
				}
			break ;
			
		case AtomicInfo::DELETED:
		case AtomicInfo::UPDATE_OLD:
		default:
			fprintf(stderr, "Bad value myEvent=%d in new tree\n", myAtomicInfo.myEvent);
			break;
		}
	
	// Now apply to Children
	
	int child = myAtomicInfo.firstChild ;
	while( child ) {
		// ConstructInsertScript( child, ancestorInserted||(myAtomicInfo.myEvent==AtomicInfo::INSERTED) );
		ConstructInsertScript( child, (myAtomicInfo.myEvent==AtomicInfo::INSERTED) );
		child = nodesManager->v1node[child].nextSibling ;
		}
	
	}

/***************************************************************
 *         ++++ Add Attributes related operations ++++         *
 ***************************************************************/

void DeltaConstructor::AddAttributeOperations( int v1nodeID ) {
	
	vddprintf(("add attr ops on node %d\n", v1nodeID ));

	xercesc_3_0::DOMNode *node = nodesManager->v1nodeByDID[ v1nodeID ] ;

	char xidstr[10] ;
	XMLCh tempStr[100];
	
	sprintf(xidstr,"%d", (int) nodesManager->v1doc->getXidMap().getXIDbyNode( node ) );

	vddprintf(("AddAttributeOperations(node::XID=%s)\n", xidstr));
	
	if (!nodesManager->v1Assigned( v1nodeID )) {
		vddprintf(("node is new, skipping\n"));
	} else {
		if (node->getNodeType()!=xercesc_3_0::DOMNode::ELEMENT_NODE) {
			vddprintf(("node is not an element, skipping\n"));
			return ;
		}
		
		int v0nodeID = nodesManager->v1node[ v1nodeID ].myMatchID ;
		xercesc_3_0::DOMNode* oldnode = nodesManager->v0nodeByDID[ v0nodeID ] ;
	
		unsigned int attLength = node->getAttributes()->getLength() ;
		for(unsigned int i=0; i<attLength; i++) {
			xercesc_3_0::DOMNode* attr = node->getAttributes()->item( i );
			xercesc_3_0::DOMNode* oldattr = oldnode->getAttributes()->getNamedItem(attr->getNodeName()) ;
			if (oldattr==NULL) {
				vddprintf(("new node %d, attribute %d inserted\n", v1nodeID, i));
				xercesc_3_0::XMLString::transcode("ai", tempStr, 99);
				xercesc_3_0::DOMElement* elem = deltaDoc->createElement(tempStr);
				xercesc_3_0::XMLString::transcode("xid", tempStr, 99);
				elem->setAttribute(tempStr, XyLatinStr(xidstr) );
				xercesc_3_0::XMLString::transcode("a", tempStr, 99);
				elem->setAttribute(tempStr,   attr->getNodeName() );
				xercesc_3_0::XMLString::transcode("v", tempStr, 99);
				elem->setAttribute(tempStr,   attr->getNodeValue() );
				scriptRoot->appendChild( elem );
			}
			else if (!xercesc_3_0::XMLString::equals(attr->getNodeValue(), oldattr->getNodeValue())) {
				vddprintf(("new node %d, attribute %d updated\n", v1nodeID, i));
				xercesc_3_0::XMLString::transcode("au", tempStr, 99);
				xercesc_3_0::DOMElement* elem = deltaDoc->createElement(tempStr);
				xercesc_3_0::XMLString::transcode("xid", tempStr, 99);
				elem->setAttribute(tempStr, XyLatinStr(xidstr) );
				xercesc_3_0::XMLString::transcode("a", tempStr, 99);
				elem->setAttribute(tempStr,   attr->getNodeName() );
				xercesc_3_0::XMLString::transcode("ov", tempStr, 99);
				elem->setAttribute(tempStr,  oldattr->getNodeValue() );
				xercesc_3_0::XMLString::transcode("nv", tempStr, 99);
				elem->setAttribute(tempStr,  attr->getNodeValue() );
				scriptRoot->appendChild( elem );
			}
			else {
				vddprintf(("attr %d from v1 has been found identical in v0\n", i));
			}
		}

		attLength = oldnode->getAttributes()->getLength() ;
		for(unsigned int i=0; i<attLength; i++) {
			xercesc_3_0::DOMNode* oldattr = oldnode->getAttributes()->item( i );
			xercesc_3_0::DOMNode* attr = node->getAttributes()->getNamedItem(oldattr->getNodeName()) ;
			if (attr==NULL) {
				vddprintf(("old node %d, attribute %d deleted\n", v0nodeID, i));
				xercesc_3_0::XMLString::transcode("ad", tempStr, 99);
				xercesc_3_0::DOMElement* elem = deltaDoc->createElement(tempStr);
				xercesc_3_0::XMLString::transcode("xid", tempStr, 99);
				elem->setAttribute(tempStr, XyLatinStr(xidstr) );
				xercesc_3_0::XMLString::transcode("a", tempStr, 99);
				elem->setAttribute(tempStr,  oldattr->getNodeName() );
				xercesc_3_0::XMLString::transcode("v", tempStr, 99);
				elem->setAttribute(tempStr,   oldattr->getNodeValue() );
				scriptRoot->appendChild( elem );
			}
		}
	}
	
	// Now apply to Children
	
	int child = nodesManager->v1node[v1nodeID].firstChild ;
	while( child ) {
		AddAttributeOperations( child );
		child = nodesManager->v1node[child].nextSibling ;
	}
	
}

/*******************************************************************
 *                                                                 *
 *                                                                 *
 * Transform the RAW structure into a Xerces DOM Tree              *
 * and XML file                                                    *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 *******************************************************************/

void DeltaConstructor::constructDeltaDocument(void) {

	vddprintf(("\n+++ Construct XML Delta Document +++\n\n")) ;

	/* ---- Clear Tables ---- */

	unsigned long long int localStart = rdtsc() ;

	int v0rootID = nodesManager->sourceNumberOfNodes ;
	int v1rootID = nodesManager->resultNumberOfNodes ;
	
	/* ---- Construct DELETE Operations script ---- */
		
	vddprintf(("\nMark old tree:\n"));
	nodesManager->MarkOldTree( v0rootID );
	
	/* ---- Construct INSERT Operations script ---- */
		
	vddprintf(("\nMark new tree:\n"));
	nodesManager->MarkNewTree( v1rootID );
		
	clocksCDDprepareVectors += rdtsc() - localStart ;

	/* ---- Save Resulting XID-map ---- */
	
	localStart = rdtsc() ;

	xercesc_3_0::DOMNode* v1rootElement = v1XML->getDocumentElement() ;
	v1XML->getXidMap().SetRootElement( v1rootElement );
	
	if (!_XyDiff_DontSaveXidmapToFile) {
		vddprintf(("\nSave resulting XID-map:\n")) ;
		std::string v1xidmapFilename = v1filename + ".xidmap" ;
		std::ofstream xidmapFile( v1xidmapFilename.c_str() ) ;
		xidmapFile << v1XML->getXidMap().String() << std::endl;
		}
	
	XMLCh tempStr[100];

	xercesc_3_0::DOMNode* tElem = deltaDoc->getDocumentElement()->getFirstChild();
	xercesc_3_0::XMLString::transcode("fromXidMap", tempStr, 99);
	xercesc_3_0::DOMAttr* v0XidMapNode = deltaDoc->createAttribute(tempStr);
	v0XidMapNode->setNodeValue(XyLatinStr(v0XML->getXidMap().String().c_str()));
	xercesc_3_0::XMLString::transcode("fromXidMap", tempStr, 99);
	xercesc_3_0::DOMAttr* v1XidMapNode = deltaDoc->createAttribute(tempStr);
	v1XidMapNode->setNodeValue(XyLatinStr(v1XML->getXidMap().String().c_str()));
	tElem->getAttributes()->setNamedItem(v0XidMapNode);
	tElem->getAttributes()->setNamedItem(v1XidMapNode);
	
	clocksCDDsaveXidMap += rdtsc() - localStart ;

	/* ---- Compute WEAK MOVE Operations ---- */
		
	localStart = rdtsc() ;

	vddprintf(("\nCompute children reordering move:\n"));
	nodesManager->ComputeWeakMove( v0rootID );
	
	clocksCDDcomputeWeakMove += rdtsc() - localStart ;
	
	/* ---- Detect UPDATE Operations ---- */
		
	localStart = rdtsc() ;

	vddprintf(("\nDetect Update operations\n"));
	nodesManager->DetectUpdate( v0rootID );
	
	clocksCDDdetectUpdate += rdtsc() - localStart ;

	/* ---- Add Attribute Operations ---- */
	
	localStart = rdtsc() ;

	vddprintf(("\nAdd Attribute operations:\n"));
	AddAttributeOperations( v1rootID );
	
	clocksCDDaddAttributeOps += rdtsc() - localStart ;
	
	/* ---- Add Delete & Insert operations to Delta ---- */
	/* (Warning) here both DOM tree are modified during the process */
		
	localStart = rdtsc() ;

	vddprintf(("\nConstruct 'delete' script:\n"));
	ConstructDeleteScript( v0rootID, false );
	
	vddprintf(("\nConstruct 'insert' script:\n"));
	ConstructInsertScript( v1rootID, false );

	if (moveCount)   throw VersionManagerException("Runtime Error", "constructDelta", "moveCount is not NULL");
	if (updateCount) throw VersionManagerException("Runtime Error", "constructDelta", "updateCount is not NULL");

	/* --- Add Rename Root Operation --- */
	
	if (!xercesc_3_0::XMLString::equals(v0XML->getDocumentElement()->getNodeName(), v1XML->getDocumentElement()->getNodeName())) {
		xercesc_3_0::DOMElement *rrOp = scriptRoot->getOwnerDocument()->createElement(xercesc_3_0::XMLString::transcode("renameRoot"));
		rrOp->setAttribute(xercesc_3_0::XMLString::transcode("from"), v0XML->getDocumentElement()->getNodeName());
		rrOp->setAttribute(xercesc_3_0::XMLString::transcode("to"), v1XML->getDocumentElement()->getNodeName());
		scriptRoot->appendChild(rrOp);
	}
	
	/* --- Done --- */

	clocksConstructDOMDelta += rdtsc() - localStart ;
	
	return ;
  }
