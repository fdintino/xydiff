#include "DeltaException.hpp"
#include "DeltaApply.hpp"
#include "DeltaSortOperations.hpp"
#include "DeltaManager.hpp"

#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"
#include "xercesc/dom/DOMAttr.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"

#include "include/XyLatinStr.hpp"
#include "include/XyInt.hpp"
#include "include/XyDelta_DOMInterface.hpp"

#include "Tools.hpp"
#include "include/XID_map.hpp"
#include <stdio.h>

static const XMLCh gLS[] = { xercesc_3_0::chLatin_L, xercesc_3_0::chLatin_S, xercesc_3_0::chNull };

XID_DOMDocument* DeltaApplyEngine::getResultDocument (void) {
  return xiddoc ;
}


/********************************************************************
 |*                                                                 *
 |* DeltaApplyEngine:: Atomic Operation implemented here            *
 |*                                                                 *
 ********************************************************************/

/* ---- MOVE FROM : Takes away a subtree in a document ---- */

void DeltaApplyEngine::Subtree_MoveFrom( XID_t myXID ) {
	vddprintf(("        move (from) node xid=%d\n", (int) myXID ));
	xercesc_3_0::DOMNode* moveNode = xiddoc->getXidMap().getNodeWithXID( myXID );
	if (moveNode==NULL) THROW_AWAY(("node with XID=%d not found",(int)myXID));
	
	xercesc_3_0::DOMNode* backupNode = moveDocument->importNode( moveNode, true );
	moveDocument->getXidMap().mapSubtree( xiddoc->getXidMap().String(moveNode).c_str() , backupNode );
	moveDocument->getDocumentElement()->appendChild( backupNode );
	
	moveNode = moveNode->getParentNode()->removeChild( moveNode );
	xiddoc->getXidMap().removeSubtree( moveNode );
	}

/* ---- DELETE a map of certain nodes in a document ---- */

void DeltaApplyEngine::Subtree_Delete( const char *xidmapStr ) {
	XidMap_Parser parse(xidmapStr) ;
	// Note here that the PostFix order for XID-map is important to garantuee that nodes will be deleted in the	correct order
	while (!parse.isListEmpty()) {
		XID_t deleteXID       = parse.getNextXID();
		vddprintf(( "        delete node xid=%d\n", (int) deleteXID ));
		xercesc_3_0::DOMNode* deleteNode   = xiddoc->getXidMap().getNodeWithXID( deleteXID );
		if (deleteNode==NULL) THROW_AWAY(("node with XID=%d not found",(int)deleteXID));
		xiddoc->getXidMap().removeNode( deleteNode );
		deleteNode            = deleteNode->getParentNode()->removeChild( deleteNode );
		}
	}

/* ---- MOVE TO: puts a subtree in the document ---- */

void DeltaApplyEngine::Subtree_MoveTo( XID_t myXID, XID_t parentXID, int position ) {
	vddprintf(("        move subtree rooted by %d to (parent=%d, pos=%d)\n", (int)myXID, (int)parentXID, position));
	
	xercesc_3_0::DOMNode* moveRoot = moveDocument->getXidMap().getNodeWithXID( myXID ) ;
	if (moveRoot==NULL) THROW_AWAY(("node with XID=%d not found",(int)myXID));
	
	Subtree_Insert(moveRoot, parentXID, position, moveDocument->getXidMap().String(moveRoot).c_str() );
	
	(void)moveRoot->getParentNode()->removeChild(moveRoot);
	}

/* ---- INSERT a map of certain nodes in a document ---- */

void DeltaApplyEngine::Subtree_Insert( xercesc_3_0::DOMNode *insertSubtreeRoot, XID_t parentXID, int position, const char *xidmapStr ) {

	vddprintf(( "        insert xidmap=%s at (parent=%d, pos=%d)\n", xidmapStr, (int)parentXID, position));
	xercesc_3_0::DOMNode* contentNode  = xiddoc->importNode( insertSubtreeRoot, true );
	xercesc_3_0::DOMNode* parentNode   = xiddoc->getXidMap().getNodeWithXID( parentXID );
	if (parentNode==NULL) THROW_AWAY(("parent node with XID=%d not found",(int)parentXID));

	int actual_pos = 1 ;
	if ((position!=1)&&(!parentNode->hasChildNodes())) THROW_AWAY(("parent has no children but position is %d",position));
	xercesc_3_0::DOMNode* brother = parentNode->getFirstChild();
	while (actual_pos < position) {
	  brother = brother->getNextSibling();
		actual_pos++;
		if ((brother==NULL)&&(actual_pos<position)) THROW_AWAY(("parent has %d children but position is %d",actual_pos-1, position));
		}
	
	// Add node to the tree
	if (brother==NULL) parentNode->appendChild( contentNode );
	else parentNode->insertBefore( contentNode, brother );
	
	xiddoc->getXidMap().mapSubtree( xidmapStr, contentNode );

	}

/* ---- UPDATE the text value of a given node ---- */

void DeltaApplyEngine::TextNode_Update( XID_t nodeXID, const XMLCh* newValue ) {
	vddprintf(("        update xid=%d\n",(int)nodeXID));
	xercesc_3_0::DOMNode* updateNode = xiddoc->getXidMap().getNodeWithXID( nodeXID );
	if (updateNode==NULL) THROW_AWAY(("node with XID=%d not found",(int)nodeXID));

	updateNode->setNodeValue( newValue ) ;
	}

/* ---- ATTRIBUTE Operations ---- */

void DeltaApplyEngine::Attribute_Insert( XID_t nodeXID, const XMLCh* attr, const XMLCh* value ) {
	vddprintf(("        insert attr at xid=%d\n",(int)nodeXID));
	xercesc_3_0::DOMNode* node = xiddoc->getXidMap().getNodeWithXID( nodeXID );
	if (node==NULL) THROW_AWAY(("node with XID=%d not found",(int)nodeXID));
	xercesc_3_0::DOMAttr* attrNode = xiddoc->createAttribute( attr );
	attrNode->setNodeValue( value );
	node->getAttributes()->setNamedItem( attrNode );
	}
	
void DeltaApplyEngine::Attribute_Update( XID_t nodeXID, const XMLCh* attr, const XMLCh* value ) {
	vddprintf(("        update attr at xid=%d\n",(int)nodeXID));
	xercesc_3_0::DOMNode* node = xiddoc->getXidMap().getNodeWithXID( nodeXID );
	if (node==NULL) THROW_AWAY(("node with XID=%d not found",(int)nodeXID));
	node->getAttributes()->getNamedItem( attr )->setNodeValue( value );
	}
	
void DeltaApplyEngine::Attribute_Delete( XID_t nodeXID, const XMLCh* attr ) {
	vddprintf(("        delete attr at xid=%d\n",(int)nodeXID));
	xercesc_3_0::DOMNode* node = xiddoc->getXidMap().getNodeWithXID( nodeXID );
	if (node==NULL) THROW_AWAY(("node with XID=%d not found",(int)nodeXID));
	node = node->getAttributes()->removeNamedItem( attr );
	}
	
/********************************************************************
 |*                                                                 *
 |* DeltaApplyEngine:: ApplyOperation                               *
 |*                                                                 *
 ********************************************************************/
	
void DeltaApplyEngine::ApplyOperation(xercesc_3_0::DOMNode *operationNode) {
	 
	vddprintf(("ApplyOperation\n"));
	XMLCh tempStr[100];
	XMLCh dStr[100];
	XMLCh iStr[100];
	XMLCh uStr[100];
	XMLCh adStr[100];
	XMLCh aiStr[100];
	XMLCh auStr[100];
	XMLCh renameRootStr[100];
	xercesc_3_0::XMLString::transcode("d", dStr, 99);
	xercesc_3_0::XMLString::transcode("i", iStr, 99);
	xercesc_3_0::XMLString::transcode("u", uStr, 99);
	xercesc_3_0::XMLString::transcode("ad", adStr, 99);
	xercesc_3_0::XMLString::transcode("ai", aiStr, 99);
	xercesc_3_0::XMLString::transcode("au", auStr, 99);
	xercesc_3_0::XMLString::transcode("renameRoot", renameRootStr, 99);
	if (xercesc_3_0::XMLString::equals(operationNode->getNodeName(), dStr)) {
		vddprintf(("        d(elete)\n"));
		
		bool move = false ;
		xercesc_3_0::XMLString::transcode("move", tempStr, 99);
		xercesc_3_0::DOMNode* moveAttr = operationNode->getAttributes()->getNamedItem(tempStr) ;
		xercesc_3_0::XMLString::transcode("yes", tempStr, 99);
		if ((moveAttr!=NULL)&&(xercesc_3_0::XMLString::equals(moveAttr->getNodeValue(),tempStr))) move=true ;

		xercesc_3_0::XMLString::transcode("xm", tempStr, 99);
		XyLatinStr xidmapStr(operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue()) ;
		
		if (move) {
			XidMap_Parser parse(xidmapStr) ;
			XID_t myXid = parse.getRootXID();
			Subtree_MoveFrom( myXid );
		  }
		else {
			Subtree_Delete(xidmapStr) ;
		  }
		}

	else if (xercesc_3_0::XMLString::equals(operationNode->getNodeName(),iStr)) {
		vddprintf(("        i(nsert)\n"));

		bool move = false ;
		xercesc_3_0::XMLString::transcode("move", tempStr, 99);
		xercesc_3_0::DOMNode* moveAttr = operationNode->getAttributes()->getNamedItem(tempStr) ;
		xercesc_3_0::XMLString::transcode("yes", tempStr, 99);
		if ((moveAttr!=NULL)&&(xercesc_3_0::XMLString::equals(moveAttr->getNodeValue(),tempStr))) move=true ;
		xercesc_3_0::XMLString::transcode("pos", tempStr, 99);
		xercesc_3_0::DOMNode *n = operationNode->getAttributes()->getNamedItem(tempStr);
		int position = XyInt(n->getNodeValue());

		xercesc_3_0::XMLString::transcode("par", tempStr, 99);
		n = operationNode->getAttributes()->getNamedItem(tempStr);
		XID_t parentXID = (XID_t)(int)XyInt(n->getNodeValue());
		
		xercesc_3_0::XMLString::transcode("xm", tempStr, 99);
		XyLatinStr xidmapStr(operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue()) ;

		if (move) {
			XidMap_Parser parse(xidmapStr) ;
			XID_t myXid = parse.getRootXID();

			Subtree_MoveTo( myXid, parentXID, position );
			}
		else {
			xercesc_3_0::DOMNode* insertRoot ;
			// get data to insert
			if (operationNode->hasChildNodes()) insertRoot = operationNode->getFirstChild() ;
			else THROW_AWAY(("insert operator element contains no data"));
				
			Subtree_Insert( insertRoot, parentXID, position, xidmapStr ) ;
			}
		}

	else if (xercesc_3_0::XMLString::equals(operationNode->getNodeName(), uStr)) {
		vddprintf(("        u(pdate)\n"));

		xercesc_3_0::XMLString::transcode("xid", tempStr, 99);
		XID_t nodeXID = (XID_t)(int)XyInt(operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue());
		xercesc_3_0::DOMNode* value = operationNode->getFirstChild() ; // Old Value node
		
		if (value==NULL) THROW_AWAY(("update operator contains no data"));

		xercesc_3_0::XMLString::transcode("nv", tempStr, 99);
		if (!xercesc_3_0::XMLString::equals(value->getNodeName(), tempStr)) value = value->getNextSibling();
		if ((value==NULL)||(!xercesc_3_0::XMLString::equals(value->getNodeName(),tempStr))) THROW_AWAY(("new-value <nv> data not found"));
		value = value->getFirstChild() ; // New Value Text node
		if ((value==NULL)||(value->getNodeType()!=xercesc_3_0::DOMNode::TEXT_NODE)) THROW_AWAY(("element <nv> does not contain text data"));

		TextNode_Update( nodeXID, value->getNodeValue() );
		}

	else if (xercesc_3_0::XMLString::equals(operationNode->getNodeName(), adStr)) {
		vddprintf(("        a(ttribute) d(elete)\n"));
		xercesc_3_0::XMLString::transcode("xid", tempStr, 99);
		XID_t nodeXID = (XID_t)(int)XyInt(operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue());

		xercesc_3_0::XMLString::transcode("a", tempStr, 99);
        const XMLCh* attr = operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue() ;
        //xercesc_3_0::DOMString attr = operationNode.getAttributes().getNamedItem("a").getNodeValue() ;
		Attribute_Delete( nodeXID, attr );
		}
	else if (xercesc_3_0::XMLString::equals(operationNode->getNodeName(), aiStr)) {
		vddprintf(("        a(ttribute) i(nsert)\n"));
		xercesc_3_0::XMLString::transcode("xid", tempStr, 99);
		XID_t nodeXID = (XID_t)(int)XyInt(operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue());

		//xercesc_3_0::DOMString attr = operationNode.getAttributes().getNamedItem("a").getNodeValue() ;
		//xercesc_3_0::DOMString value = operationNode.getAttributes().getNamedItem("v").getNodeValue() ;
		xercesc_3_0::XMLString::transcode("a", tempStr, 99);
        const XMLCh* attr = operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue() ;
		xercesc_3_0::XMLString::transcode("v", tempStr, 99);
        const XMLCh* value = operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue() ;
		Attribute_Insert( nodeXID, attr, value );
		}
	else if (xercesc_3_0::XMLString::equals(operationNode->getNodeName(), auStr)) {
		vddprintf(("        a(ttribute) u(pdate)\n"));
		xercesc_3_0::XMLString::transcode("xid", tempStr, 99);
		XID_t nodeXID = (XID_t)(int)XyInt(operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue());

		//xercesc_3_0::DOMString attr = operationNode.getAttributes().getNamedItem("a").getNodeValue() ;
		//xercesc_3_0::DOMString value = operationNode.getAttributes().getNamedItem("nv").getNodeValue() ;
		xercesc_3_0::XMLString::transcode("a", tempStr, 99);
		const XMLCh* attr = operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue() ;
		xercesc_3_0::XMLString::transcode("nv", tempStr, 99);
        const XMLCh* value = operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue() ;
		Attribute_Update( nodeXID, attr, value );
		}
	else if (xercesc_3_0::XMLString::equals(operationNode->getNodeName(), renameRootStr)) {
		vddprintf(("        renameRoot\n"));
		xercesc_3_0::DOMNode *root = xiddoc->getDocumentElement();
		XID_t rootXID = xiddoc->getXidMap().getXIDbyNode(root);
		xercesc_3_0::XMLString::transcode("to", tempStr, 99);
		const XMLCh* newrootName = operationNode->getAttributes()->getNamedItem(tempStr)->getNodeValue();
		xercesc_3_0::DOMElement* newroot = xiddoc->createElement(newrootName);
		xercesc_3_0::DOMNode* child = root->getFirstChild();
		while(child!=NULL) {
			root->removeChild(child);
			newroot->appendChild(child);
			child = root->getFirstChild();
		}
		xercesc_3_0::DOMNamedNodeMap *attributes = root->getAttributes();
		for(unsigned int i=0;i<attributes->getLength();i++) {
			xercesc_3_0::DOMNode *an = attributes->item(i);
			newroot->setAttribute(an->getNodeName(), an->getNodeValue());
		}
		xiddoc->removeChild(root);
		xiddoc->getXidMap().removeNode(root);
		xiddoc->appendChild(newroot);
		xiddoc->getXidMap().registerNode(newroot, rootXID);
		xiddoc->getXidMap().SetRootElement(newroot);
		}
	}

/********************************************************************
 |*                                                                 *
 |*                                                                 *
 |* DeltaApply                                                      *
 |*                                                                 *
 |*                                                                 *
 ********************************************************************/

void DeltaApplyEngine::ApplyDeltaElement(xercesc_3_0::DOMNode* incDeltaElement) {
	vddprintf(("Apply delta element\n"));
	deltaElement = incDeltaElement;
	
	/* ---- Do All DELETE Operations ( including 'from' part of move ) ---- */
	
	vddprintf(("Apply Delete operations\n"));
	xercesc_3_0::DOMNode* firstOp = deltaElement->getFirstChild() ;
	vddprintf(("    first sort delete operations...\n"));
	SortDeleteOperationsEngine deleteOps(xiddoc, firstOp);
	vddprintf(("    then apply all of them 1 by 1...\n"));
	while(!deleteOps.isListEmpty()) {
		xercesc_3_0::DOMNode* op=deleteOps.getNextDeleteOperation();
		ApplyOperation(op);
		}

	vddprintf(("Ok, there are no more delete operations.\n"));
	
	/* ---- Do All INSERT Operations ( including 'to' part of move ) ---- */
	
	firstOp = deltaElement->getFirstChild() ;
	SortInsertOperationsEngine insertOps(xiddoc, firstOp);
	while(!insertOps.isListEmpty()) {
		xercesc_3_0::DOMNode* op=insertOps.getNextInsertOperation();
		ApplyOperation(op);
		}
	
	/* ---- Do all  UPDATE  &  ATTRIBUTE  Operations ---- */

	xercesc_3_0::DOMNode* child = deltaElement->getFirstChild() ;
	XMLCh iStr[100];
	xercesc_3_0::XMLString::transcode("i", iStr, 99);
	XMLCh dStr[100];
	xercesc_3_0::XMLString::transcode("d", dStr, 99);
	while (child != NULL) {
		if ( (!xercesc_3_0::XMLString::equals(child->getNodeName(),iStr))
		
		   &&(!xercesc_3_0::XMLString::equals(child->getNodeName(), dStr)) ) ApplyOperation(child);
	  child = child->getNextSibling() ;
		}
		
	/* ---- Small consistency checks ---- */

	if (moveDocument->getDocumentElement()->hasChildNodes()) THROW_AWAY(("temporary document used to move node is not empty!"));

	vddprintf(("xiddoc=%s\n",xiddoc->getXidMap().String().c_str() ));
}

void DeltaApplyEngine::ApplyDelta(XID_DOMDocument *IncDeltaDoc) {
	
	deltaElement      = DeltaApplyEngine::getDeltaElement(IncDeltaDoc);
	deltaDoc          = IncDeltaDoc ;
	
	vddprintf(("FROM: %s\n", DeltaApplyEngine::getSourceURI(IncDeltaDoc).c_str() ));
	vddprintf(("TO:   %s\n", DeltaApplyEngine::getDestinationURI(IncDeltaDoc).c_str() ));

	ApplyDeltaElement(deltaElement);
}



// for applying more delta elements (backwardNumber) one time

void DeltaApplyEngine::ApplyDelta(XID_DOMDocument *IncDeltaDoc, int backwardNumber) {
    
	xercesc_3_0::DOMNode* deltaRoot = IncDeltaDoc->getDocumentElement();  // <delta_unit>
	xercesc_3_0::DOMNode* deltaElement ;
	XMLCh tempStr[100];
	xercesc_3_0::XMLString::transcode("", tempStr, 99);
		for ( deltaElement = deltaRoot->getLastChild(); backwardNumber > 0;  deltaElement = deltaElement->getPreviousSibling(), backwardNumber-- ) {
                  xercesc_3_0::DOMImplementation* impl =  xercesc_3_0::DOMImplementationRegistry::getDOMImplementation(gLS);
                  xercesc_3_0::DOMDocument* backwardDeltaDoc = impl->createDocument(0, tempStr,0);
                
                  //xercesc_3_0::DOMDocument* backwardDeltaDoc = xercesc_3_0::DOMDocument::createDocument();
                  xercesc_3_0::DOMNode* backwardDeltaElement = XyDelta::ReverseDelta(backwardDeltaDoc, deltaElement);			
			ApplyDeltaElement(backwardDeltaElement);
		}
	
}

DeltaApplyEngine::DeltaApplyEngine(XID_DOMDocument *sourceDoc) {
  
	if (sourceDoc    == NULL)          THROW_AWAY(("document is null"));
	xiddoc            = sourceDoc ;

	vddprintf(("creating temporary move document\n"));
	moveDocument = XID_DOMDocument::createDocument() ;
	moveDocument->initEmptyXidmapStartingAt(-1);

	XMLCh tempStr[100];
	xercesc_3_0::XMLString::transcode("moveRoot", tempStr, 99);
	xercesc_3_0::DOMElement* moveRoot = moveDocument->createElement(tempStr) ;
	moveDocument->appendChild( moveRoot );
	}

xercesc_3_0::DOMNode* DeltaApplyEngine::getDeltaElement(XID_DOMDocument *IncDeltaDoc) {

	if (IncDeltaDoc  == NULL) THROW_AWAY(("delta document is null"));
	//xercesc_3_0::DOMNode* dRoot    = IncDeltaDoc->getDocumentElement();
	xercesc_3_0::DOMNode* dRoot = IncDeltaDoc->getElementsByTagName(xercesc_3_0::XMLString::transcode("unit_delta"))->item(0);
	XMLCh tempStr[100];
	xercesc_3_0::XMLString::transcode("unit_delta", tempStr, 99);
	if ( (dRoot==NULL)
	   ||(!xercesc_3_0::XMLString::equals(dRoot->getNodeName(), tempStr))) THROW_AWAY(("no <unit_delta> root found on document"));

	xercesc_3_0::XMLString::transcode("t", tempStr, 99);
//	const XMLCh * firstChildNodeName = dRoot->getFirstChild()->getNodeName();
//	char * firstChildNodeChar = xercesc_3_0::XMLString::transcode(firstChildNodeName);
	xercesc_3_0::DOMNodeList * tElementNodes = IncDeltaDoc->getElementsByTagName(xercesc_3_0::XMLString::transcode("t"));
	if (tElementNodes->getLength() == 0) THROW_AWAY(("no delta element <t> found"));
//	   ||(!xercesc_3_0::XMLString::equals(dRoot->getFirstChild()->getNodeName(), tempStr))) THROW_AWAY(("no delta element <t> found"));
	
//	xercesc_3_0::DOMNode* deltaElement = dRoot->getFirstChild() ;
	xercesc_3_0::DOMNode* deltaElement = tElementNodes->item(0);
	return deltaElement ;
	}


std::string DeltaApplyEngine::getSourceURI(XID_DOMDocument *IncDeltaDoc) {
	XMLCh tempStr[100];
	
	xercesc_3_0::DOMNode* deltaElement = DeltaApplyEngine::getDeltaElement(IncDeltaDoc);
	xercesc_3_0::XMLString::transcode("from", tempStr, 99);
	xercesc_3_0::DOMNode* fromItem = deltaElement->getAttributes()->getNamedItem(tempStr);
	if (fromItem==NULL) THROW_AWAY(("attribute 'from' not found"));
	
        return std::string(XyLatinStr(fromItem->getNodeValue()));
	}
	
std::string DeltaApplyEngine::getDestinationURI(XID_DOMDocument *IncDeltaDoc) {
	XMLCh tempStr[100];

	xercesc_3_0::DOMNode* deltaElement = DeltaApplyEngine::getDeltaElement(IncDeltaDoc);
	xercesc_3_0::XMLString::transcode("to", tempStr, 99);
	xercesc_3_0::DOMNode* toItem = deltaElement->getAttributes()->getNamedItem(tempStr);
	if (toItem==NULL) THROW_AWAY(("attribute 'to' not found"));
	
        return std::string(XyLatinStr(toItem->getNodeValue()));
	}


