#include "XyDiff/DeltaException.hpp"
#include "XyDiff/DeltaApply.hpp"
#include "XyDiff/DeltaSortOperations.hpp"
#include "XyDiff/DeltaManager.hpp"

#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"
#include "xercesc/dom/DOMAttr.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/XMLUniDefs.hpp"

#include "XyDiff/include/XyLatinStr.hpp"
#include "XyDiff/include/XyInt.hpp"
#include "XyDiff/include/XyDelta_DOMInterface.hpp"

#include "XyDiff/Tools.hpp"
#include "XyDiff/include/XID_map.hpp"
#include <stdio.h>

static const XMLCh gLS[] = { xercesc_2_2::chLatin_L, xercesc_2_2::chLatin_S, xercesc_2_2::chNull };

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
	xercesc_2_2::DOMNode* moveNode = xiddoc->getXidMap().getNodeWithXID( myXID );
	if (moveNode==NULL) THROW_AWAY(("node with XID=%d not found",(int)myXID));
	
	xercesc_2_2::DOMNode* backupNode = moveDocument->importNode( moveNode, true );
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
		xercesc_2_2::DOMNode* deleteNode   = xiddoc->getXidMap().getNodeWithXID( deleteXID );
		if (deleteNode==NULL) THROW_AWAY(("node with XID=%d not found",(int)deleteXID));
		xiddoc->getXidMap().removeNode( deleteNode );
		deleteNode            = deleteNode->getParentNode()->removeChild( deleteNode );
		}
	}

/* ---- MOVE TO: puts a subtree in the document ---- */

void DeltaApplyEngine::Subtree_MoveTo( XID_t myXID, XID_t parentXID, int position ) {
	vddprintf(("        move subtree rooted by %d to (parent=%d, pos=%d)\n", (int)myXID, (int)parentXID, position));
	
	xercesc_2_2::DOMNode* moveRoot = moveDocument->getXidMap().getNodeWithXID( myXID ) ;
	if (moveRoot==NULL) THROW_AWAY(("node with XID=%d not found",(int)myXID));
	
	Subtree_Insert(moveRoot, parentXID, position, moveDocument->getXidMap().String(moveRoot).c_str() );
	
	(void)moveRoot->getParentNode()->removeChild(moveRoot);
	}

/* ---- INSERT a map of certain nodes in a document ---- */

void DeltaApplyEngine::Subtree_Insert( xercesc_2_2::DOMNode *insertSubtreeRoot, XID_t parentXID, int position, const char *xidmapStr ) {

	vddprintf(( "        insert xidmap=%s at (parent=%d, pos=%d)\n", xidmapStr, (int)parentXID, position));
	xercesc_2_2::DOMNode* contentNode  = xiddoc->importNode( insertSubtreeRoot, true );
	xercesc_2_2::DOMNode* parentNode   = xiddoc->getXidMap().getNodeWithXID( parentXID );
	if (parentNode==NULL) THROW_AWAY(("parent node with XID=%d not found",(int)parentXID));

	int actual_pos = 1 ;
	if ((position!=1)&&(!parentNode->hasChildNodes())) THROW_AWAY(("parent has no children but position is %d",position));
	xercesc_2_2::DOMNode* brother = parentNode->getFirstChild();
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
	xercesc_2_2::DOMNode* updateNode = xiddoc->getXidMap().getNodeWithXID( nodeXID );
	if (updateNode==NULL) THROW_AWAY(("node with XID=%d not found",(int)nodeXID));

	updateNode->setNodeValue( newValue ) ;
	}

/* ---- ATTRIBUTE Operations ---- */

void DeltaApplyEngine::Attribute_Insert( XID_t nodeXID, const XMLCh* attr, const XMLCh* value ) {
	vddprintf(("        insert attr at xid=%d\n",(int)nodeXID));
	xercesc_2_2::DOMNode* node = xiddoc->getXidMap().getNodeWithXID( nodeXID );
	if (node==NULL) THROW_AWAY(("node with XID=%d not found",(int)nodeXID));
	xercesc_2_2::DOMAttr* attrNode = xiddoc->createAttribute( attr );
	attrNode->setNodeValue( value );
	node->getAttributes()->setNamedItem( attrNode );
	}
	
void DeltaApplyEngine::Attribute_Update( XID_t nodeXID, const XMLCh* attr, const XMLCh* value ) {
	vddprintf(("        update attr at xid=%d\n",(int)nodeXID));
	xercesc_2_2::DOMNode* node = xiddoc->getXidMap().getNodeWithXID( nodeXID );
	if (node==NULL) THROW_AWAY(("node with XID=%d not found",(int)nodeXID));
	node->getAttributes()->getNamedItem( attr )->setNodeValue( value );
	}
	
void DeltaApplyEngine::Attribute_Delete( XID_t nodeXID, const XMLCh* attr ) {
	vddprintf(("        delete attr at xid=%d\n",(int)nodeXID));
	xercesc_2_2::DOMNode* node = xiddoc->getXidMap().getNodeWithXID( nodeXID );
	if (node==NULL) THROW_AWAY(("node with XID=%d not found",(int)nodeXID));
	node = node->getAttributes()->removeNamedItem( attr );
	}
	
/********************************************************************
 |*                                                                 *
 |* DeltaApplyEngine:: ApplyOperation                               *
 |*                                                                 *
 ********************************************************************/
	
void DeltaApplyEngine::ApplyOperation(xercesc_2_2::DOMNode *operationNode) {
	 
	vddprintf(("ApplyOperation\n"));
		
	if (xercesc_2_2::XMLString::equals(operationNode->getNodeName(), L"d")) {
		vddprintf(("        d(elete)\n"));
		
		bool move = false ;
		xercesc_2_2::DOMNode* moveAttr = operationNode->getAttributes()->getNamedItem(L"move") ;
		if ((moveAttr!=NULL)&&(xercesc_2_2::XMLString::equals(moveAttr->getNodeValue(),L"yes"))) move=true ;
			
		XyLatinStr xidmapStr(operationNode->getAttributes()->getNamedItem(L"xm")->getNodeValue()) ;
		
		if (move) {
			XidMap_Parser parse(xidmapStr) ;
			XID_t myXid = parse.getRootXID();
			Subtree_MoveFrom( myXid );
		  }
		else {
			Subtree_Delete(xidmapStr) ;
		  }
		}

	else if (xercesc_2_2::XMLString::equals(operationNode->getNodeName(),L"i")) {
		vddprintf(("        i(nsert)\n"));

		bool move = false ;
		xercesc_2_2::DOMNode* moveAttr = operationNode->getAttributes()->getNamedItem(L"move") ;
		if ((moveAttr!=NULL)&&(xercesc_2_2::XMLString::equals(moveAttr->getNodeValue(),L"yes"))) move=true ;
		
		xercesc_2_2::DOMNode *n = operationNode->getAttributes()->getNamedItem(L"pos");
		int position = XyInt(n->getNodeValue());
		
		n = operationNode->getAttributes()->getNamedItem(L"par");
		XID_t parentXID = (XID_t)(int)XyInt(n->getNodeValue());
		
		XyLatinStr xidmapStr(operationNode->getAttributes()->getNamedItem(L"xm")->getNodeValue()) ;

		if (move) {
			XidMap_Parser parse(xidmapStr) ;
			XID_t myXid = parse.getRootXID();

			Subtree_MoveTo( myXid, parentXID, position );
			}
		else {
			xercesc_2_2::DOMNode* insertRoot ;
			// get data to insert
			if (operationNode->hasChildNodes()) insertRoot = operationNode->getFirstChild() ;
			else THROW_AWAY(("insert operator element contains no data"));
				
			Subtree_Insert( insertRoot, parentXID, position, xidmapStr ) ;
			}
		}

	else if (xercesc_2_2::XMLString::equals(operationNode->getNodeName(), L"u")) {
		vddprintf(("        u(pdate)\n"));
		
		XID_t nodeXID = (XID_t)(int)XyInt(operationNode->getAttributes()->getNamedItem(L"xid")->getNodeValue());
		xercesc_2_2::DOMNode* value = operationNode->getFirstChild() ; // Old Value node
		
		if (value==NULL) THROW_AWAY(("update operator contains no data"));
		if (!xercesc_2_2::XMLString::equals(value->getNodeName(), L"nv")) value = value->getNextSibling();
		if ((value==NULL)||(!xercesc_2_2::XMLString::equals(value->getNodeName(),L"nv"))) THROW_AWAY(("new-value <nv> data not found"));
		value = value->getFirstChild() ; // New Value Text node
		if ((value==NULL)||(value->getNodeType()!=xercesc_2_2::DOMNode::TEXT_NODE)) THROW_AWAY(("element <nv> does not contain text data"));

		TextNode_Update( nodeXID, value->getNodeValue() );
		}

	else if (xercesc_2_2::XMLString::equals(operationNode->getNodeName(), L"ad")) {
		vddprintf(("        a(ttribute) d(elete)\n"));
		XID_t nodeXID = (XID_t)(int)XyInt(operationNode->getAttributes()->getNamedItem(L"xid")->getNodeValue());
		
                const XMLCh* attr = operationNode->getAttributes()->getNamedItem(L"a")->getNodeValue() ;
                //xercesc_2_2::DOMString attr = operationNode.getAttributes().getNamedItem("a").getNodeValue() ;
		Attribute_Delete( nodeXID, attr );
		}
	else if (xercesc_2_2::XMLString::equals(operationNode->getNodeName(), L"ai")) {
		vddprintf(("        a(ttribute) i(nsert)\n"));
		XID_t nodeXID = (XID_t)(int)XyInt(operationNode->getAttributes()->getNamedItem(L"xid")->getNodeValue());

		//xercesc_2_2::DOMString attr = operationNode.getAttributes().getNamedItem("a").getNodeValue() ;
		//xercesc_2_2::DOMString value = operationNode.getAttributes().getNamedItem("v").getNodeValue() ;
                const XMLCh* attr = operationNode->getAttributes()->getNamedItem(L"a")->getNodeValue() ;
                const XMLCh* value = operationNode->getAttributes()->getNamedItem(L"v")->getNodeValue() ;
		Attribute_Insert( nodeXID, attr, value );
		}
	else if (xercesc_2_2::XMLString::equals(operationNode->getNodeName(), L"au")) {
		vddprintf(("        a(ttribute) u(pdate)\n"));
		XID_t nodeXID = (XID_t)(int)XyInt(operationNode->getAttributes()->getNamedItem(L"xid")->getNodeValue());

		//xercesc_2_2::DOMString attr = operationNode.getAttributes().getNamedItem("a").getNodeValue() ;
		//xercesc_2_2::DOMString value = operationNode.getAttributes().getNamedItem("nv").getNodeValue() ;
                const XMLCh* attr = operationNode->getAttributes()->getNamedItem(L"a")->getNodeValue() ;
                const XMLCh* value = operationNode->getAttributes()->getNamedItem(L"nv")->getNodeValue() ;
		Attribute_Update( nodeXID, attr, value );
		}
	else if (xercesc_2_2::XMLString::equals(operationNode->getNodeName(), L"renameRoot")) {
		vddprintf(("        renameRoot\n"));
		xercesc_2_2::DOMNode *root = xiddoc->getDocumentElement();
		XID_t rootXID = xiddoc->getXidMap().getXIDbyNode(root);
		const XMLCh* newrootName = operationNode->getAttributes()->getNamedItem(L"to")->getNodeValue();
		xercesc_2_2::DOMElement* newroot = xiddoc->createElement(newrootName);
		xercesc_2_2::DOMNode* child = root->getFirstChild();
		while(child!=NULL) {
			root->removeChild(child);
			newroot->appendChild(child);
			child = root->getFirstChild();
		}
		xercesc_2_2::DOMNamedNodeMap *attributes = root->getAttributes();
		for(unsigned int i=0;i<attributes->getLength();i++) {
			xercesc_2_2::DOMNode *an = attributes->item(i);
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

void DeltaApplyEngine::ApplyDeltaElement(xercesc_2_2::DOMNode* incDeltaElement) {
	vddprintf(("Apply delta element\n"));
	deltaElement = incDeltaElement;
	
	/* ---- Do All DELETE Operations ( including 'from' part of move ) ---- */
	
	vddprintf(("Apply Delete operations\n"));
	xercesc_2_2::DOMNode* firstOp = deltaElement->getFirstChild() ;
	vddprintf(("    first sort delete operations...\n"));
	SortDeleteOperationsEngine deleteOps(xiddoc, firstOp);
	vddprintf(("    then apply all of them 1 by 1...\n"));
	while(!deleteOps.isListEmpty()) {
		xercesc_2_2::DOMNode* op=deleteOps.getNextDeleteOperation();
		ApplyOperation(op);
		}

	vddprintf(("Ok, there are no more delete operations.\n"));
	
	/* ---- Do All INSERT Operations ( including 'to' part of move ) ---- */
	
	firstOp = deltaElement->getFirstChild() ;
	SortInsertOperationsEngine insertOps(xiddoc, firstOp);
	while(!insertOps.isListEmpty()) {
		xercesc_2_2::DOMNode* op=insertOps.getNextInsertOperation();
		ApplyOperation(op);
		}
	
	/* ---- Do all  UPDATE  &  ATTRIBUTE  Operations ---- */

	xercesc_2_2::DOMNode* child = deltaElement->getFirstChild() ;
	while (child != NULL) {
          if ( (!xercesc_2_2::XMLString::equals(child->getNodeName(),L"i"))
		   &&(!xercesc_2_2::XMLString::equals(child->getNodeName(), L"d")) ) ApplyOperation(child);
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
    
	xercesc_2_2::DOMNode* deltaRoot = IncDeltaDoc->getDocumentElement();  // <delta_unit>
	xercesc_2_2::DOMNode* deltaElement ;
	
		for ( deltaElement = deltaRoot->getLastChild(); backwardNumber > 0;  deltaElement = deltaElement->getPreviousSibling(), backwardNumber-- ) {
                  xercesc_2_2::DOMImplementation* impl =  xercesc_2_2::DOMImplementationRegistry::getDOMImplementation(gLS);
                  xercesc_2_2::DOMDocument* backwardDeltaDoc = impl->createDocument(0, L"",0);
                
                  //xercesc_2_2::DOMDocument* backwardDeltaDoc = xercesc_2_2::DOMDocument::createDocument();
                  xercesc_2_2::DOMNode* backwardDeltaElement = XyDelta::ReverseDelta(backwardDeltaDoc, deltaElement);			
			ApplyDeltaElement(backwardDeltaElement);
		}
	
}

DeltaApplyEngine::DeltaApplyEngine(XID_DOMDocument *sourceDoc) {
  
	if (sourceDoc    == NULL)          THROW_AWAY(("document is null"));
	xiddoc            = sourceDoc ;

	vddprintf(("creating temporary move document\n"));
	moveDocument = XID_DOMDocument::createDocument() ;
	moveDocument->initEmptyXidmapStartingAt(-1);

	xercesc_2_2::DOMElement* moveRoot = moveDocument->createElement(L"moveRoot") ;
	moveDocument->appendChild( moveRoot );
	}

xercesc_2_2::DOMNode* DeltaApplyEngine::getDeltaElement(XID_DOMDocument *IncDeltaDoc) {

	if (IncDeltaDoc  == NULL) THROW_AWAY(("delta document is null"));
	xercesc_2_2::DOMNode* dRoot    = IncDeltaDoc->getDocumentElement();

	if ( (dRoot==NULL)
	   ||(!xercesc_2_2::XMLString::equals(dRoot->getNodeName(), L"unit_delta"))) THROW_AWAY(("no <unit_delta> root found on document"));

	if ( (!dRoot->hasChildNodes())
	   ||(!xercesc_2_2::XMLString::equals(dRoot->getFirstChild()->getNodeName(), L"t"))) THROW_AWAY(("no delta element <t> found"));

	xercesc_2_2::DOMNode* deltaElement = dRoot->getFirstChild() ;
	return deltaElement ;
	}


std::string DeltaApplyEngine::getSourceURI(XID_DOMDocument *IncDeltaDoc) {

	xercesc_2_2::DOMNode* deltaElement = DeltaApplyEngine::getDeltaElement(IncDeltaDoc);
	xercesc_2_2::DOMNode* fromItem = deltaElement->getAttributes()->getNamedItem(L"from");
	if (fromItem==NULL) THROW_AWAY(("attribute 'from' not found"));
	
        return std::string(XyLatinStr(fromItem->getNodeValue()));
	}
	
std::string DeltaApplyEngine::getDestinationURI(XID_DOMDocument *IncDeltaDoc) {

	xercesc_2_2::DOMNode* deltaElement = DeltaApplyEngine::getDeltaElement(IncDeltaDoc);
	xercesc_2_2::DOMNode* toItem = deltaElement->getAttributes()->getNamedItem(L"to");
	if (toItem==NULL) THROW_AWAY(("attribute 'to' not found"));
	
        return std::string(XyLatinStr(toItem->getNodeValue()));
	}


