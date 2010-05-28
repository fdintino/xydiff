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

	v0XML        = fromXDD ;
	v1XML        = toXDD ;
	v0filename   = from ;
	v1filename   = to ;
	nodesManager = incMappings ;
	
	moveCount    = 0 ;
	updateCount  = 0 ;
	
	ignoreUnimportantData = IncIgnoreUnimportantData ;
	XMLCh tempStrA[100];
	XMLCh tempStrB[100];

	XMLCh *xmlnsURI_ch = XMLString::transcode("http://www.w3.org/2000/xmlns/");
	XMLCh *xmlns_ch = XMLString::transcode("xmlns");

	XMLString::transcode("urn:schemas-xydiff:unit-delta", xyDiffNS_ch, 99);
	deltaDoc     = XID_DOMDocument::createDocument() ;


	XMLString::transcode("unit_delta", tempStrA, 99);
	DOMElement* deltaRoot = deltaDoc->createElement(tempStrA);
	deltaRoot->setAttributeNS(xmlnsURI_ch, xmlns_ch, xyDiffNS_ch);
	deltaDoc->appendChild( (DOMNode*) deltaRoot );

	XMLString::transcode("t", tempStrA, 99);
	DOMElement* tElem = deltaDoc->createElement(tempStrA);

	if (ignoreUnimportantData) {
		XMLString::transcode("IgnoreBlankTexts", tempStrA, 99);
		XMLString::transcode("yes", tempStrB, 99);
		tElem->setAttribute(tempStrA, tempStrB);
	}
	deltaRoot->appendChild( tElem );

	// Copy namespaces from root element
	DOMNamedNodeMap *v1XMLAttrs = v1XML->getDocumentElement()->getAttributes();
	XMLSize_t i;
	for (i = 0; i < v1XMLAttrs->getLength(); i++) {
		DOMAttr *attr = (DOMAttr *) v1XMLAttrs->item(i);
		// If it is prefixed with xmlns
		if (XMLString::compareNString(attr->getName(), xmlns_ch, 5) == 0) {
			DOMNamedNodeMap *deltaAttrs = deltaRoot->getAttributes();
			// If we don't already have this attribute
			if (deltaAttrs->getNamedItem( attr->getName() ) == NULL) {
				deltaRoot->setAttributeNS(xmlnsURI_ch, attr->getName(), attr->getValue());
			}
		}
	}
	DOMNamedNodeMap *v0XMLAttrs = v0XML->getDocumentElement()->getAttributes();
	for (i = 0; i < v0XMLAttrs->getLength(); i++) {
		DOMAttr *attr = (DOMAttr *) v0XMLAttrs->item(i);
		// If it is prefixed with xmlns
		if (XMLString::compareNString(attr->getName(), xmlns_ch, 5) == 0) {
			DOMNamedNodeMap *deltaAttrs = deltaRoot->getAttributes();
			// If we don't already have this attribute
			if (deltaAttrs->getNamedItem( attr->getName() ) == NULL) {
				deltaRoot->setAttributeNS(xmlnsURI_ch, attr->getName(), attr->getValue());
			}
		}
	}
	
	
	scriptRoot = tElem ;

	XMLString::release(&xmlnsURI_ch);
	XMLString::release(&xmlns_ch);

	}

/*****************************************************
 *         ++++ Construct Delete Script ++++         *
 *****************************************************/

void DeltaConstructor::ConstructDeleteScript( int v0nodeID, bool ancestorDeleted ) {

	class AtomicInfo & myAtomicInfo = nodesManager->v0node[ v0nodeID ] ;
	DOMNode* node = nodesManager->v0nodeByDID[ v0nodeID ] ;

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
				XMLCh *parXID_str_ch = XMLString::transcode(parXID_str);
				XMLCh *pos_str_ch    = XMLString::transcode(pos_str);
				XMLCh *myXidMap_ch   = XMLString::transcode(myXidMap.c_str());

				XMLString::transcode("d", tempStrA, 99);
				DOMElement* elem = deltaDoc->createElement(tempStrA);
				XMLString::transcode( "par", tempStrA, 99 );
				elem->setAttribute( tempStrA, parXID_str_ch );
				XMLString::transcode( "pos", tempStrA, 99 );
				elem->setAttribute( tempStrA, pos_str_ch );
				XMLString::transcode( "xm", tempStrA, 99 );
				elem->setAttribute( tempStrA, myXidMap_ch );

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
						XMLString::transcode("move", tempStrA, 99);
						XMLString::transcode("yes", tempStrB, 99);
						elem->setAttribute(tempStrA, tempStrB);
						moveCount++ ;
						}
						break;
					case AtomicInfo::UPDATE_OLD: {
						DOMNode* contentNode = deltaDoc->importNode( node, true );
						elem->appendChild ( contentNode );
						XMLString::transcode("update", tempStrA, 99);
						XMLString::transcode("yes", tempStrB, 99);
						elem->setAttribute(tempStrA, tempStrB);
						updateCount++ ;
						// Add update tag
						XMLString::transcode("u", tempStrA, 99);
						upElem = deltaDoc->createElement(tempStrA);
						XMLString::transcode( "par", tempStrA, 99 );
						upElem->setAttribute( tempStrA, parXID_str_ch );
						XMLString::transcode( "pos", tempStrA, 99 );
						upElem->setAttribute( tempStrA, pos_str_ch );
						XMLString::transcode( "oldxm", tempStrA, 99 );
						upElem->setAttribute( tempStrA, myXidMap_ch );
						upElem->appendChild(elem);
						
						}
						break;
					default:
						XMLString::release(&parXID_str_ch);
						XMLString::release(&pos_str_ch);
						XMLString::release(&myXidMap_ch);
						throw VersionManagerException(
							"Internal Error",
							"ConstructDeleteScript",
							"Program can't possibly be here"
						);
					}
				XMLString::release(&parXID_str_ch);
				XMLString::release(&pos_str_ch);
				XMLString::release(&myXidMap_ch);

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
	XMLCh tempStrA[100];
	XMLCh tempStrB[100];
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
				XMLCh *parXID_str_ch = XMLString::transcode(parXID_str);
				XMLCh *pos_str_ch = XMLString::transcode(pos_str);

				XMLString::transcode("i", tempStrA, 99);
				DOMElement* elem = deltaDoc->createElement(tempStrA);
				XMLString::transcode("par", tempStrA, 99);
				elem->setAttribute(tempStrA, parXID_str_ch);
				XMLString::transcode("pos", tempStrA, 99);
				elem->setAttribute(tempStrA, pos_str_ch);

				DOMNode *upElem;
				switch(myAtomicInfo.myEvent) {
					case AtomicInfo::INSERTED: {
						// DOM_Node contentNode = deltaDoc.importNode( node, true );
						std::vector<XID_t> xidList ;
						DOMNode* contentNode = deltaDoc_ImportInsertTree( v1nodeID, xidList );
						elem->appendChild ( contentNode );

						XMLString::transcode("xm", tempStrA, 99);
						XMLCh *xidListStringCh = XMLString::transcode(nodesManager->v1doc->getXidMap().StringFromList(xidList).c_str());
						elem->setAttribute(tempStrA, xidListStringCh );
						XMLString::release(&xidListStringCh);
						}
						break;
					case AtomicInfo::STRONGMOVE:
					case AtomicInfo::WEAKMOVE: {
						// Erase moved subtree so that any parent inserted operation won't take it
                        // DOMNode* movedNode = parentNode->removeChild( node );
                        parentNode->removeChild( node );
						XMLString::transcode("move", tempStrA, 99);
						XMLString::transcode("yes", tempStrB, 99);
						elem->setAttribute(tempStrA, tempStrB);
						moveCount-- ;
			 			char xidstr[10] ;
						sprintf(xidstr,"(%d)", (int)nodesManager->v1doc->getXidMap().getXIDbyNode( node ));
						XMLCh *xidstr_ch = XMLString::transcode(xidstr);
						XMLString::transcode("xm", tempStrA, 99);
						elem->setAttribute(tempStrA,  xidstr_ch );
						XMLString::release(&xidstr_ch);
						}
						break;
					case AtomicInfo::UPDATE_NEW: {
						// Find associated update element created in ConstructDeleteScript()
						XMLString::transcode("u", tempStrA, 99);
						DOMNodeList *upNodes = ((DOMElement *)scriptRoot)->getElementsByTagName(tempStrA);
						if (upNodes->getLength() ==0) {
							THROW_AWAY(("Could not find any matching <u> elements for insert\n"));
						}
						for (XMLSize_t i = 0; i < upNodes->getLength(); i++) {
							DOMNode *tmpUpNode = upNodes->item(i);
							XMLString::transcode("par", tempStrA, 99);
							XMLString::transcode("pos", tempStrB, 99);
							
							if (XMLString::equals(parXID_str_ch, ((DOMElement*)tmpUpNode)->getAttribute(tempStrA))
							    && XMLString::equals(pos_str_ch, ((DOMElement*)tmpUpNode)->getAttribute(tempStrB))
							) {
								upElem = tmpUpNode;
							}
						}

						std::vector<XID_t> xidList ;
						DOMNode* contentNode = deltaDoc_ImportInsertTree( v1nodeID, xidList );
						elem->appendChild ( contentNode );

						XMLString::transcode("xm", tempStrA, 99);
						XMLCh *xidListStringCh = XMLString::transcode(nodesManager->v1doc->getXidMap().StringFromList(xidList).c_str());
						elem->setAttribute(tempStrA, xidListStringCh );
						XMLString::release(&xidListStringCh);

						XMLString::transcode("update", tempStrA, 99);
						XMLString::transcode("yes", tempStrB, 99);
						elem->setAttribute(tempStrA, tempStrB);	
						
						vddprintf(("found %i matching update nodes\n", upNodes->getLength()));
						if (upElem != NULL) {
							DOMNode *delElem = upElem->getFirstChild();
							XMLString::transcode("d", tempStrA, 99);
							if (XMLString::compareString(delElem->getNodeName(), tempStrA) != 0) {
								char *delElemNodeName = XMLString::transcode(delElem->getNodeName());
								char *error_msg = NULL;
								sprintf(error_msg, "<u> element has incorrect first child <%s>, should be <d>\n", delElemNodeName);
								XMLString::release(&delElemNodeName);
								THROW_AWAY((error_msg));
							}

							if (delElem->getFirstChild()->getNodeValue() != NULL && contentNode->getNodeValue() != NULL) {
								XyStrDiff *strdiff = new XyStrDiff(deltaDoc, (DOMElement *)upElem, delElem->getFirstChild()->getNodeValue(), contentNode->getNodeValue());
								strdiff->LevenshteinDistance();
								delete strdiff;
							}

							XMLString::transcode("newxm", tempStrA, 99);
							XMLCh *xidListStringCh = XMLString::transcode(nodesManager->v1doc->getXidMap().StringFromList(xidList).c_str());
							((DOMElement*)upElem)->setAttribute(tempStrA, xidListStringCh );
							XMLString::release(&xidListStringCh);

							upElem->removeChild(delElem);
							delElem->release();
						}
						
						updateCount-- ;
						}
						break;
					default:
						XMLString::release(&parXID_str_ch);
						XMLString::release(&pos_str_ch);
						throw VersionManagerException(
							"Internal Error",
							"ConstructDeleteScript",
							"Program can't possibly be here"
						);
					}
				XMLString::release(&parXID_str_ch);
				XMLString::release(&pos_str_ch);

				if ((!ignoreUnimportantData)||(!myAtomicInfo.isUnimportant)) {
					// Update elements function differently, they have already been
					// appended in DeltaConstructor::ConstructDeleteScript()
					if (myAtomicInfo.myEvent != AtomicInfo::UPDATE_NEW) {
						scriptRoot->appendChild( elem );
					} else {
						elem->release();
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
	XMLCh tempStrA[100];
	XMLCh tempStrB[100];
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
		unsigned int i;
		for(i=0; i<attLength; i++) {
			DOMNode* attr = node->getAttributes()->item( i );
			DOMNode* oldattr = oldnode->getAttributes()->getNamedItem(attr->getNodeName()) ;
			if (oldattr==NULL) {
				vddprintf(("new node %d, attribute %d inserted\n", v1nodeID, i));
				XMLString::transcode("ai", tempStrA, 99);
				DOMElement* elem = deltaDoc->createElement(tempStrA);
				XMLString::transcode("xid", tempStrA, 99);
				XMLString::transcode(xidstr, tempStrB, 99);
				elem->setAttribute(tempStrA, tempStrB);
				XMLString::transcode("a", tempStrA, 99);
				elem->setAttribute(tempStrA, attr->getNodeName() );
				XMLString::transcode("v", tempStrA, 99);
				elem->setAttribute(tempStrA, attr->getNodeValue() );
				scriptRoot->appendChild( elem );
			}
			else if (!XMLString::equals(attr->getNodeValue(), oldattr->getNodeValue())) {
				vddprintf(("new node %d, attribute %d updated\n", v1nodeID, i));
				XMLString::transcode("au", tempStrA, 99);
				DOMElement* elem = deltaDoc->createElement(tempStrA);
				XMLString::transcode("xid", tempStrA, 99);
				XMLString::transcode(xidstr, tempStrB, 99);
				elem->setAttribute(tempStrA, tempStrB);
				XMLString::transcode("a", tempStrA, 99);
				elem->setAttribute(tempStrA, attr->getNodeName() );
				XMLString::transcode("ov", tempStrA, 99);
				elem->setAttribute(tempStrA, oldattr->getNodeValue() );
				XMLString::transcode("nv", tempStrA, 99);
				elem->setAttribute(tempStrA, attr->getNodeValue() );
				scriptRoot->appendChild( elem );
			}
			else {
				vddprintf(("attr %d from v1 has been found identical in v0\n", i));
			}
		}

		attLength = oldnode->getAttributes()->getLength() ;
		for(i=0; i<attLength; i++) {
			DOMNode* oldattr = oldnode->getAttributes()->item( i );
			DOMNode* attr = node->getAttributes()->getNamedItem(oldattr->getNodeName()) ;
			if (attr==NULL) {
				vddprintf(("old node %d, attribute %d deleted\n", v0nodeID, i));
				XMLString::transcode("ad", tempStrA, 99);
				DOMElement* elem = deltaDoc->createElement(tempStrA);
				XMLString::transcode("xid", tempStrA, 99);
				XMLString::transcode(xidstr, tempStrB, 99);
				elem->setAttribute(tempStrA, tempStrB);
				XMLString::transcode("a", tempStrA, 99);
				elem->setAttribute(tempStrA, oldattr->getNodeName() );
				XMLString::transcode("v", tempStrA, 99);
				elem->setAttribute(tempStrA, oldattr->getNodeValue() );
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


	size_t v0rootID = nodesManager->sourceNumberOfNodes ;
	size_t v1rootID = nodesManager->resultNumberOfNodes ;
	
	/* ---- Construct DELETE Operations script ---- */
		
	vddprintf(("\nMark old tree:\n"));
	nodesManager->MarkOldTree( v0rootID );
	
	/* ---- Construct INSERT Operations script ---- */
		
	vddprintf(("\nMark new tree:\n"));
	nodesManager->MarkNewTree( v1rootID );
		

	/* ---- Save Resulting XID-map ---- */
	

	DOMNode* v1rootElement = v1XML->getDocumentElement() ;
	v1XML->getXidMap().SetRootElement( v1rootElement );
	
	if (!_XyDiff_DontSaveXidmapToFile) {
		vddprintf(("\nSave resulting XID-map:\n")) ;
		std::string v1xidmapFilename = v1filename + ".xidmap" ;
		std::ofstream xidmapFile( v1xidmapFilename.c_str() ) ;
		xidmapFile << v1XML->getXidMap().String() << std::endl;
		}

	DOMNode* tElem = deltaDoc->getDocumentElement()->getFirstChild();
	XMLCh *fromXidMap_ch  = XMLString::transcode("fromXidMap");
	XMLCh *v0XidMapStr_ch = XMLString::transcode(v0XML->getXidMap().String().c_str());
	XMLCh *v1XidMapStr_ch = XMLString::transcode(v1XML->getXidMap().String().c_str());
	DOMAttr* v0XidMapNode = deltaDoc->createAttribute(fromXidMap_ch);
	v0XidMapNode->setNodeValue(v0XidMapStr_ch);
	DOMAttr* v1XidMapNode = deltaDoc->createAttribute(fromXidMap_ch);
	v1XidMapNode->setNodeValue(v1XidMapStr_ch);
	tElem->getAttributes()->setNamedItem(v0XidMapNode);
	tElem->getAttributes()->setNamedItem(v1XidMapNode);
	XMLString::release(&fromXidMap_ch);
	XMLString::release(&v0XidMapStr_ch);
	XMLString::release(&v1XidMapStr_ch);

	/* ---- Compute WEAK MOVE Operations ---- */
		

	vddprintf(("\nCompute children reordering move:\n"));
	nodesManager->ComputeWeakMove( v0rootID );
	
	
	/* ---- Detect UPDATE Operations ---- */
		

	vddprintf(("\nDetect Update operations\n"));
	nodesManager->DetectUpdate( v0rootID );
	

	/* ---- Add Attribute Operations ---- */
	

	vddprintf(("\nAdd Attribute operations:\n"));
	AddAttributeOperations( v1rootID );
	
	
	/* ---- Add Delete & Insert operations to Delta ---- */
	/* (Warning) here both DOM tree are modified during the process */
		

	vddprintf(("\nConstruct 'delete' script:\n"));
	ConstructDeleteScript( v0rootID, false );
	
	vddprintf(("\nConstruct 'insert' script:\n"));
	ConstructInsertScript( v1rootID, false );

	if (moveCount)   throw VersionManagerException("Runtime Error", "constructDelta", "moveCount is not NULL");
	if (updateCount) throw VersionManagerException("Runtime Error", "constructDelta", "updateCount is not NULL");

	/* --- Add Rename Root Operation --- */
	if (!XMLString::equals(v0XML->getDocumentElement()->getNodeName(), v1XML->getDocumentElement()->getNodeName())) {
		XMLCh tempStr[100];
		XMLString::transcode("renameRoot", tempStr, 99);
		DOMElement *rrOp = scriptRoot->getOwnerDocument()->createElement(tempStr);
		XMLString::transcode("from", tempStr, 99);
		rrOp->setAttribute(tempStr, v0XML->getDocumentElement()->getNodeName());
		XMLString::transcode("to", tempStr, 99);
		rrOp->setAttribute(tempStr, v1XML->getDocumentElement()->getNodeName());
		scriptRoot->appendChild(rrOp);
	}
	
	/* --- Done --- */

	
	return ;
  }
