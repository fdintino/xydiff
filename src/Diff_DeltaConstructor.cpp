#include "include/XyLatinStr.hpp"
#include "include/XyUTF8Str.hpp"
#include "Diff_DeltaConstructor.hpp"

#include "CommonSubSequenceAlgorithms.hpp"
#include "Tools.hpp"
#include "include/XID_map.hpp"
#include "Diff_NodesManager.hpp"
#include "include/XyStrDiff.hpp"
#include "DeltaException.hpp"

#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
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

XERCES_CPP_NAMESPACE_USE

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

	DOMElement* deltaRoot = deltaDoc->createElement(XMLString::transcode("unit_delta"));
	deltaRoot->setAttributeNS(
		XMLString::transcode("http://www.w3.org/2000/xmlns/"),
		XMLString::transcode("xmlns:xyd"),
		XMLString::transcode("http://www.nflc.org/schema/xydiff/"));
	deltaDoc->appendChild( (DOMNode*) deltaRoot );

	DOMElement* tElem = deltaDoc->createElement(XMLString::transcode("t"));

	tElem->setAttribute(XMLString::transcode("from"), XyLatinStr(v0filename.c_str()) );

	tElem->setAttribute(XMLString::transcode("to"), XyLatinStr(v1filename.c_str()) );

	if (ignoreUnimportantData) tElem->setAttribute(XMLString::transcode("IgnoreBlankTexts"),XMLString::transcode("yes"));
	deltaRoot->appendChild( tElem );

	scriptRoot = tElem ;
	
	clocksCDDinitDomDelta += rdtsc() - localStart ;
	}

/*****************************************************
 *         ++++ Construct Delete Script ++++         *
 *****************************************************/

void DeltaConstructor::ConstructDeleteScript( int v0nodeID, bool ancestorDeleted ) {

	class AtomicInfo & myAtomicInfo = nodesManager->v0node[ v0nodeID ] ;
	DOMNode* node = nodesManager->v0nodeByDID[ v0nodeID ] ;


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
				vddprintf((
					"skipping op 'delete node %d' because subtree is deleted starting from one of its ancestor\n",
					myAtomicInfo.myID
				));
			}
			else {
				DOMNode* parentNode = node->getParentNode() ;
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
				DOMElement* elem = deltaDoc->createElement(XMLString::transcode("d"));
				elem->setAttribute(XMLString::transcode("par"), XyLatinStr( parXID_str) );
				elem->setAttribute(XMLString::transcode("pos"), XyLatinStr(pos_str) );
				elem->setAttribute(XMLString::transcode("xm"),  XyLatinStr( myXidMap.c_str()) );
				DOMElement *upElem;
				switch(myAtomicInfo.myEvent) {
					case AtomicInfo::DELETED: {
						DOMNode* contentNode = deltaDoc->importNode( node, true );
						elem->appendChild ( contentNode );
						}
						break;
					case AtomicInfo::STRONGMOVE:
					case AtomicInfo::WEAKMOVE: {
						// Erase moved subtree so that any parent deleted operation won't take it
						//DOMNode* movedNode = parentNode->removeChild( node );
						parentNode->removeChild( node );
						elem->setAttribute(XMLString::transcode("move"), XMLString::transcode("yes"));
						moveCount++ ;
						}
						break;
					case AtomicInfo::UPDATE_OLD: {
						DOMNode* contentNode = deltaDoc->importNode( node, true );
						elem->appendChild ( contentNode );
						elem->setAttribute(XMLString::transcode("update"), XMLString::transcode("yes"));
						updateCount++ ;
						// Add update tag
						upElem = deltaDoc->createElement(XMLString::transcode("u"));
						upElem->setAttribute(XMLString::transcode("par"), XyLatinStr( parXID_str) );
						upElem->setAttribute(XMLString::transcode("pos"), XyLatinStr(pos_str) );
						upElem->setAttribute(XMLString::transcode("oldxm"), XyLatinStr( myXidMap.c_str()));
						upElem->appendChild(elem);
						
						}
						break;
					default:
						throw VersionManagerException(
							"Internal Error",
							"ConstructDeleteScript",
							"Program can't possibly be here"
						);
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
					if (myAtomicInfo.myEvent == AtomicInfo::UPDATE_OLD) {
						scriptRoot->appendChild( upElem );
					} else {
						scriptRoot->appendChild( elem );
					}
					
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

DOMNode* DeltaConstructor::deltaDoc_ImportInsertTree( int v1nodeID, std::vector<XID_t> &xidList ) {
	class AtomicInfo & myAtomicInfo = nodesManager->v1node[ v1nodeID ] ;
	
	DOMNode* node = nodesManager->v1nodeByDID[ v1nodeID ] ;
	DOMNode* newNode = deltaDoc->importNode( node, false );
	
	// Now apply to Children
	
	int child = myAtomicInfo.firstChild ;
	while( child ) {
		class AtomicInfo & myChildInfo = nodesManager->v1node[child] ;
		
		// If Children is to be inserted
		if ((myChildInfo.myEvent==AtomicInfo::INSERTED)
		     ||(myChildInfo.myEvent==AtomicInfo::UPDATE_NEW)) {
			DOMNode* childNode = deltaDoc_ImportInsertTree(child, xidList);
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
	DOMNode* node = nodesManager->v1nodeByDID[ v1nodeID ] ;

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
				vddprintf((
					"skipping op 'insert node %d' because subtree is inserted starting from one of its ancestor\n",
					myAtomicInfo.myID
				));
				}

			else {
				DOMNode* parentNode = node->getParentNode() ;
				XID_t parentXID = nodesManager->v1doc->getXidMap().getXIDbyNode( parentNode );
				int myPosition = myAtomicInfo.myPosition ; // getPosition( parentNode, node );

				char parXID_str[10] ;
				char pos_str[10] ;
				sprintf(parXID_str,"%d", (int)parentXID );
				sprintf(pos_str, "%d", myPosition);

				DOMElement* elem = deltaDoc->createElement(XMLString::transcode("i"));
				elem->setAttribute(XMLString::transcode("par"), XyLatinStr(parXID_str) );
				elem->setAttribute(XMLString::transcode("pos"), XyLatinStr(pos_str) );

				DOMNode *upElem;
				switch(myAtomicInfo.myEvent) {
					case AtomicInfo::INSERTED: {
						// DOM_Node contentNode = deltaDoc.importNode( node, true );
						std::vector<XID_t> xidList ;
						DOMNode* contentNode = deltaDoc_ImportInsertTree( v1nodeID, xidList );
						elem->appendChild ( contentNode );
						elem->setAttribute(
							XMLString::transcode("xm"),
							XyLatinStr(nodesManager->v1doc->getXidMap().StringFromList(xidList).c_str()) );
						}
						break;
					case AtomicInfo::STRONGMOVE:
					case AtomicInfo::WEAKMOVE: {
						// Erase moved subtree so that any parent inserted operation won't take it
                        // DOMNode* movedNode = parentNode->removeChild( node );
                        parentNode->removeChild( node );
						elem->setAttribute(XMLString::transcode("move"), XMLString::transcode("yes"));
						moveCount-- ;
			 			char xidstr[10] ;
						sprintf(xidstr,"(%d)", (int)nodesManager->v1doc->getXidMap().getXIDbyNode( node ));
						elem->setAttribute(XMLString::transcode("xm"),  XyLatinStr(xidstr) );
						}
						break;
					case AtomicInfo::UPDATE_NEW: {
						// Find associated update element created in ConstructDeleteScript()
						DOMNodeList *upNodes = ((DOMElement *)scriptRoot)->getElementsByTagName(XMLString::transcode("u"));
						if (upNodes->getLength() ==0) {
							THROW_AWAY(("Could not find any matching <u> elements for insert\n"));
						}
						for (int i = 0; i < upNodes->getLength(); i++) {
							DOMNode *tmpUpNode = upNodes->item(i);
							if (XMLString::equals(XyLatinStr(parXID_str), ((DOMElement*)tmpUpNode)->getAttribute(XMLString::transcode("par")))
							    && XMLString::equals(XyLatinStr(pos_str), ((DOMElement*)tmpUpNode)->getAttribute(XMLString::transcode("pos")))
							) {
								upElem = tmpUpNode;
							}
						}

						std::vector<XID_t> xidList ;
						DOMNode* contentNode = deltaDoc_ImportInsertTree( v1nodeID, xidList );
						elem->appendChild ( contentNode );
						elem->setAttribute(
							XMLString::transcode("xm"),
							XyLatinStr(nodesManager->v1doc->getXidMap().StringFromList(xidList).c_str()) );
						elem->setAttribute(XMLString::transcode("update"), XMLString::transcode("yes"));
						
						
						vddprintf(("found %i matching update nodes\n", upNodes->getLength()));
						if (upElem != NULL) {
							DOMNode *delElem = upElem->getFirstChild();
							if (XMLString::compareString(
								 delElem->getNodeName(),
								 XMLString::transcode("d")
							  ) != 0)
							{
								THROW_AWAY((
									"<u> element has incorrect first child <%s>, should be <d>\n",
									XMLString::transcode(delElem->getNodeName())));
							}

							char *oldValue = XMLString::transcode(delElem->getFirstChild()->getNodeValue());
							char *newValue = XMLString::transcode(contentNode->getNodeValue());

							if (oldValue != NULL && newValue != NULL) {
								XyStrDiff *strdiff = new XyStrDiff(deltaDoc, (DOMElement *)upElem, oldValue, newValue);
								strdiff->LevenshteinDistance();
							}

							((DOMElement*)upElem)->setAttribute(
							   XMLString::transcode("newxm"),
							   XyLatinStr(nodesManager->v1doc->getXidMap().StringFromList(xidList).c_str()) );
							upElem->removeChild(delElem);
							delElem->release();
						}
						
						updateCount-- ;
						}
						break;
					default:
						throw VersionManagerException(
							"Internal Error",
							"ConstructDeleteScript",
							"Program can't possibly be here"
						);
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
					// Update elements function differently, they have already been
					// appended in DeltaConstructor::ConstructDeleteScript()
					if (myAtomicInfo.myEvent != AtomicInfo::UPDATE_NEW) {
						scriptRoot->appendChild( elem );
					}
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

	DOMNode *node = nodesManager->v1nodeByDID[ v1nodeID ] ;

	char xidstr[10] ;
	
	sprintf(xidstr,"%d", (int) nodesManager->v1doc->getXidMap().getXIDbyNode( node ) );

	vddprintf(("AddAttributeOperations(node::XID=%s)\n", xidstr));
	
	if (!nodesManager->v1Assigned( v1nodeID )) {
		vddprintf(("node is new, skipping\n"));
	} else {
		if (node->getNodeType()!=DOMNode::ELEMENT_NODE) {
			vddprintf(("node is not an element, skipping\n"));
			return ;
		}
		
		int v0nodeID = nodesManager->v1node[ v1nodeID ].myMatchID ;
		DOMNode* oldnode = nodesManager->v0nodeByDID[ v0nodeID ] ;
	
		unsigned int attLength = node->getAttributes()->getLength() ;
		for(unsigned int i=0; i<attLength; i++) {
			DOMNode* attr = node->getAttributes()->item( i );
			DOMNode* oldattr = oldnode->getAttributes()->getNamedItem(attr->getNodeName()) ;
			if (oldattr==NULL) {
				vddprintf(("new node %d, attribute %d inserted\n", v1nodeID, i));
				DOMElement* elem = deltaDoc->createElement(XMLString::transcode("ai"));
				elem->setAttribute(XMLString::transcode("xid"), XyLatinStr(xidstr) );
				elem->setAttribute(XMLString::transcode("a"), attr->getNodeName() );
				elem->setAttribute(XMLString::transcode("v"), attr->getNodeValue() );
				scriptRoot->appendChild( elem );
			}
			else if (!XMLString::equals(attr->getNodeValue(), oldattr->getNodeValue())) {
				vddprintf(("new node %d, attribute %d updated\n", v1nodeID, i));
				DOMElement* elem = deltaDoc->createElement(XMLString::transcode("au"));
				elem->setAttribute(XMLString::transcode("xid"), XyLatinStr(xidstr) );
				elem->setAttribute(XMLString::transcode("a"),   attr->getNodeName() );
				elem->setAttribute(XMLString::transcode("ov"),  oldattr->getNodeValue() );
				elem->setAttribute(XMLString::transcode("nv"),  attr->getNodeValue() );
				scriptRoot->appendChild( elem );
			}
			else {
				vddprintf(("attr %d from v1 has been found identical in v0\n", i));
			}
		}

		attLength = oldnode->getAttributes()->getLength() ;
		for(unsigned int i=0; i<attLength; i++) {
			DOMNode* oldattr = oldnode->getAttributes()->item( i );
			DOMNode* attr = node->getAttributes()->getNamedItem(oldattr->getNodeName()) ;
			if (attr==NULL) {
				vddprintf(("old node %d, attribute %d deleted\n", v0nodeID, i));
				DOMElement* elem = deltaDoc->createElement(XMLString::transcode("ad"));
				elem->setAttribute(XMLString::transcode("xid"), XyLatinStr(xidstr) );
				elem->setAttribute(XMLString::transcode("a"),  oldattr->getNodeName() );
				elem->setAttribute(XMLString::transcode("v"),   oldattr->getNodeValue() );
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

	DOMNode* v1rootElement = v1XML->getDocumentElement() ;
	v1XML->getXidMap().SetRootElement( v1rootElement );
	
	if (!_XyDiff_DontSaveXidmapToFile) {
		vddprintf(("\nSave resulting XID-map:\n")) ;
		std::string v1xidmapFilename = v1filename + ".xidmap" ;
		std::ofstream xidmapFile( v1xidmapFilename.c_str() ) ;
		xidmapFile << v1XML->getXidMap().String() << std::endl;
		}

	DOMNode* tElem = deltaDoc->getDocumentElement()->getFirstChild();
	DOMAttr* v0XidMapNode = deltaDoc->createAttribute(XMLString::transcode("fromXidMap"));
	v0XidMapNode->setNodeValue(XyLatinStr(v0XML->getXidMap().String().c_str()));
	DOMAttr* v1XidMapNode = deltaDoc->createAttribute(XMLString::transcode("fromXidMap"));
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
	
	if (!XMLString::equals(v0XML->getDocumentElement()->getNodeName(), v1XML->getDocumentElement()->getNodeName())) {
		DOMElement *rrOp = scriptRoot->getOwnerDocument()->createElement(XMLString::transcode("renameRoot"));
		rrOp->setAttribute(XMLString::transcode("from"), v0XML->getDocumentElement()->getNodeName());
		rrOp->setAttribute(XMLString::transcode("to"), v1XML->getDocumentElement()->getNodeName());
		scriptRoot->appendChild(rrOp);
	}
	
	/* --- Done --- */

	clocksConstructDOMDelta += rdtsc() - localStart ;
	
	return ;
  }
